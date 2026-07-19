/* $Id: dreamliner_poe.c,v 1.4 2017/07/14 02:51:38 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/dreamliner_poe.c,v $
 *------------------------------------------------------------------
 *
 * dreamliner_poe.c - This file contains functions for Dreamliner POE 
 *                    controller.
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2017 by Cisco Systems, Inc.
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
#include "platform_fru.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h> 

#include "dreamliner.h"
#include "dreamliner_ge_switch.h"
#include "dreamliner_fpga.h"
#include "dreamliner_poe.h"

#define POE_DEBUG 1
/* Structure for POE controller registers test */
/*  name, offset, type, S2W addr,  mask, default value */

static const reg_info_t poe_reg_test_tbl[] = {
    {"Timing Config",       ILP_TIMING_CONFIG,     READ_WRITE, {1}, 0xFF, 0x00},
    {"Misc Config",         ILP_MISC_CONFIG,       READ_WRITE, {1}, 0x8C, 0x00},
    {"end",                 0x00,                  0,          {0}, 0,    0},
};

extern int phy_detect_phone(int port);
extern boolean has_poe_psu(uint32_t);
extern boolean is_juno(void);
extern uint32_t check_poe_psu_present(uint32_t, uint32_t);

boolean poe_si_flg; 
int poe_init();
static int peek_poe_reg();
static int poke_poe_reg();
static int display_poe_reg();
static int poe_2x_mode();
int poe_power_ports_util();
static int dl_power_detect();
static int poe_display_env();
void poe_si_flag_write(char *);
boolean poe_si_flag_read(void);

/* submenu for POE utilities */
submenu_xtable_t poe_util_submenu_table[] = {
    {"peek POE register",  
     (PFT)peek_poe_reg,           0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"poke POE register",  
     (PFT)poke_poe_reg,           0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"display POE registers",  
     (PFT)display_poe_reg,        0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"POE 2X mode",  
     (PFT)poe_2x_mode,            0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"Ports power detection (Cisco PD)",  
     (PFT)dl_power_detect,        0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"Power on/off ports",  
     (PFT)poe_power_ports_util,   0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"Display ports current/voltage",  
     (PFT)poe_display_env,        0, 0, (type_t(*)())0, 0, 
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

/*********************************************************************
 *
 * Function: poe_utils()
 *
 * Description: Build the primary & secondary submenus for the
 * POE utility menu. 
 *
 * Inputs: none       
 * Outputs: PASSED
 *
 *********************************************************************
 */
int 
poe_utils ()
{
    uchar device_id0 = 0;
    poe_si_flag_write("FALSE");

    /* Read Device ID of both chips */
    if (dl_read_i2c(POE_I2C_ADDR0, ILP_ID, 1, &device_id0) == FAILED) {
        return (FAILED);
    }    
    if ((device_id0 & 0xf0) == TI_MSR_ID) {
        poe_si_flag_write("FALSE");
        printf("\nPoE chip is TI, Device ID = %x ", device_id0);
        fflush(0);
    } else if ((device_id0 & 0xf0) == SI_MSR_ID) {
        poe_si_flag_write("TRUE");
        printf("\nPoE chip is SI, Device ID = %x ", device_id0);
        fflush(0);
    } else {
        printf("\nPoE chip invalid , Device ID = %x ", device_id0);
    }         
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nI2C addr 0x%x Device ID 0x%x\n", POE_I2C_ADDR0, device_id0);
        fflush(0);
    }

    build_primary_submenu(poe_util_submenu_table, 
			  POE_UTIL_SUBMENU_TABLE_SIZE,
			  "POE Utility", &poe_submenup);
    build_secondary_submenu(poe_util_submenu_table,
			    POE_UTIL_SUBMENU_TABLE_SIZE,
			    poe_util_secondary_items);
    menu(&poe_util_menu, poe_util_secondary_items, '\0');
    
    return PASSED;
}


/*******************************************************************************
 *
 * Function: poe_reg_show()
 *
 * This function prints the poe register values.
 *
 * Input: 
 *        i2c_addr
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int
poe_reg_show (uchar i2c_addr)
{
    uchar val = 0;
    uint offset;

    printf("\nILP i2c_addr %#x\n", i2c_addr);

    offset = ILP_INTERRUPT;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_INTERRUPT               reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_INT_MASK;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_INT_MASK                reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_POWER_EVENT;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_POWER_EVENT             reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_POWER_EVENT_COR;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_POWER_EVENT_COR         reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_DETECT_EVENT;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_DETECT_EVENT            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_DETECT_EVENT_COR;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_DETECT_EVENT_COR        reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_FAULT_EVENT;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_FAULT_EVENT             reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_FAULT_EVENT_COR;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_FAULT_EVENT_COR         reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_TSTART_EVENT;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_TSTART_EVENT            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_TSTART_EVENT_COR;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_TSTART_EVENT_COR        reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_SUPPLY_EVENT;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_SUPPLY_EVENT            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_SUPPLY_EVENT_COR;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_SUPPLY_EVENT_COR        reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PORT1_STATUS;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_PORT1_STATUS            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PORT2_STATUS;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_PORT2_STATUS            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PORT3_STATUS;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_PORT3_STATUS            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PORT4_STATUS;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_PORT4_STATUS            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_POWER_STATUS;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_POWER_STATUS            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PIN_STATUS;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_PIN_STATUS              reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_OPERATING_MODE;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_OPERATING_MODE          reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_DISCONNECT_ENABLE;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_DISCONNECT_ENABLE       reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_DETECT_CLASS_ENABLE;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_DETECT_CLASS_ENABLE     reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PWRPR_ICUT_DISABLE;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_PWRPR_ICUT_DISABLE      reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_TIMING_CONFIG;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_TIMING_CONFIG           reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_MISC_CONFIG;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_MISC_CONFIG             reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_DET_CLASS_RESTART_PB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_DET_CLASS_RESTART_PB    reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_POWER_ENABLE_PB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_POWER_ENABLE_PB         reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_GLOBAL_PB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_GLOBAL_PB               reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_ID;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_ID                      reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_POLICE_21_CONFIG;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_POLICE_21_CONFIG        reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_POLICE_43_CONFIG;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_POLICE_43_CONFIG        reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IEEE_PWR_ENABLE;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_IEEE_PWR_ENABLE         reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PWR_ON_FAULT;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_PWR_ON_FAULT            reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_TEMPERATURE;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_TEMPERATURE             reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_INPUT_VDC_LSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_INPUT_VDC_LSB           reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_INPUT_VDC_MSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_INPUT_VDC_MSB           reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P1_LSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_IDC_P1_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P1_MSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_IDC_P1_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P1_LSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_VDC_P1_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P1_MSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_VDC_P1_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P2_LSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_IDC_P2_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P2_MSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_IDC_P2_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P2_LSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_VDC_P2_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P2_MSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_VDC_P2_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P3_LSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_IDC_P3_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P3_MSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_IDC_P3_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P3_LSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_VDC_P3_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P3_MSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_VDC_P3_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P4_LSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_IDC_P4_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P4_MSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_IDC_P4_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P4_LSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_VDC_P4_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P4_MSB;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_VDC_P4_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_2X_MODE;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_2X_MODE                 reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_FIRMWARE_REV;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_FIRMWARE_REV            reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_WATCHDOG;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_WATCHDOG                reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_DEVICE_ID;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_DEVICE_ID               reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_COOL_DOWN;
    dl_read_i2c(i2c_addr, offset, 1, &val);
    printf("ILP_COOL_DOWN               reg %#.2x = %#.2x\n", offset, val);
    
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
    if (is_daughter_card_present() == FALSE) {
        printf("The POE daughter card is not installed\n");
        return (FAILED);
    }

    
#if POE_DEBUG
    if ((check_poe_psu_present(POE_PSU_ONE, QUICK_MODE) == FALSE) && 
        (check_poe_psu_present(POE_PSU_TWO, QUICK_MODE) == FALSE)) {
#else
    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
#endif
        printf("\n WARNING, WARNING, Skipping ILP Register Display.\n"
               " ILP power supply may not be installed.\n");
        printf(" If ILP power supply is present then there is an ISSUE.\n"
               " Please check the power supply and ILP modules.\n");
        return (PASSED);
    }

    /* 8 POE ports */
    poe_reg_show(POE_I2C_ADDR0);

    if (poe_si_flag_read() == TRUE) {
        poe_reg_show(POE_I2C_ADDR2);
    } else {
    poe_reg_show(POE_I2C_ADDR1);
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
    uchar i2c_addr, reg, data = 0;
    int retval=FAILED;

    if (is_daughter_card_present() == FALSE) {
        printf("The POE daughter card is not installed\n");
        return (PASSED);
    }

#if POE_DEBUG
    if ((check_poe_psu_present(POE_PSU_ONE, QUICK_MODE) == FALSE) && 
        (check_poe_psu_present(POE_PSU_TWO, QUICK_MODE) == FALSE)) {
#else
    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
#endif
        printf("\n WARNING, WARNING, Skipping ILP Register Reading.\n"
               " ILP power supply may not installed.\n");
        printf(" If ILP power supply is present then there is an ISSUE.\n"
               " Please check the power supply and ILP modules.\n");
        return (PASSED);
    }

    i2c_addr = gethex_answer("Enter the i2c address(POE0:0x20, POE1:0x28 or POE1:0x29): ", 
			     POE_I2C_ADDR0, POE_I2C_ADDR0, POE_I2C_ADDR2);
    reg = gethex_answer("Enter the register offset: ", 0, 0, 0x45);

    retval = dl_read_i2c(i2c_addr, reg, 1, &data);
    if (retval == PASSED) {
        printf("peek POE register: i2c_addr %#.2x, reg offset %#.2x, data %#.2x\n",
                i2c_addr, reg, data);
    } else {
        printf("Can not read POE register.\n");
    }
    
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
    uchar i2c_addr, reg, data = 0;
    int retval=FAILED;

    if (is_daughter_card_present() == FALSE) {
        printf("The POE daughter card is not installed\n");
        return (PASSED);
    }

#if POE_DEBUG
    if ((check_poe_psu_present(POE_PSU_ONE, QUICK_MODE) == FALSE) && 
        (check_poe_psu_present(POE_PSU_TWO, QUICK_MODE) == FALSE)) {
#else
    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
#endif
        printf("\n WARNING, WARNING, Skipping ILP Register Altering.\n"
               " ILP power supply may not installed.\n");
        printf(" If ILP power supply is present then there is an ISSUE.\n"
               " Please check the power supply and ILP modules.\n");
        return (PASSED);
    }

    i2c_addr = gethex_answer("Enter the i2c address(POE0:0x20, POE1:0x28 or POE1:0x29): ", 
			     POE_I2C_ADDR0, POE_I2C_ADDR0, POE_I2C_ADDR2);
    reg = gethex_answer("Enter the register offset: ", 0, 0, 0x45);
    data = gethex_answer("Enter the data: ", 0, 0, 0xFF);

    retval = dl_write_i2c(i2c_addr, reg, 1, &data);
    if (retval != PASSED) {
        printf("Alter ILP reg write I2C failed.\n");
    }

    return (retval);
}

/* ******************************************************************
 *
 * Function: poe_2x_mode_enable
 *
 * Description: Enable or Disable PoE 2x mode (support 802.3at, 
 *               30W power) in Dreamliner.
 *
 * Input:   
 *           port_num - Face plate port num 
 *           enable - 1 to enable, 0 to disable.
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
static int poe_2x_mode_enable (int port_num, uchar enable)
{
    int ilp_port_num;
    uchar i2c_addr = 0, wr_data, icut_status = 0, power_2x_status = 0;

    if (port_num < DREAMLINER_4GE_PHY_PORTS) {
        i2c_addr = POE_I2C_ADDR0;
    } else if (port_num < DREAMLINER_8GE_PHY_PORTS) {
        if (poe_si_flag_read() == TRUE) {
            i2c_addr = POE_I2C_ADDR2;
        } else {
            i2c_addr = POE_I2C_ADDR1;
        }
    }
    ilp_port_num = port_num % POE_PORTS;

    /* Set the 2x power plus (0x40) */
    if (dl_read_i2c(i2c_addr, ILP_2X_MODE, 1,
		    &power_2x_status) == FAILED) {
        return (FAILED);
    }
    if (enable) {
	wr_data = power_2x_status | (POWER_2X << ilp_port_num);
    } else {
	wr_data = power_2x_status & ~(POWER_2X << ilp_port_num);
    }

    if (dl_write_i2c(i2c_addr, ILP_2X_MODE, 1,
		     &wr_data) == FAILED) {
        return (FAILED);
    }

    if ((ilp_port_num == 0) || (ilp_port_num == 1)) {
         /* Set police 21 configuration register(0x1e) */
        if (dl_read_i2c(i2c_addr, ILP_POLICE_21_CONFIG, 1,
			&icut_status) == FAILED) {
            return (FAILED);
        }
        if (enable){
	    wr_data = icut_status | (0x07 << (ilp_port_num % 2) * 4);
        } else { /* disable */
	    wr_data = icut_status | (ICUT_MAX << (ilp_port_num % 2) * 4);
        }
        if (dl_write_i2c(i2c_addr, ILP_POLICE_21_CONFIG, 1,
			 &wr_data) == FAILED) {
            return (FAILED);
        }
    } else { 
        /* Set police 43 configuration register (0x1f) */
        if (dl_read_i2c(i2c_addr, ILP_POLICE_43_CONFIG, 1,
			&icut_status) == FAILED) {
            return (FAILED);
        }
        if (enable){
	    wr_data = icut_status | (0x07 << (ilp_port_num % 2) * 4);
        } else { /* disable */
	    wr_data = icut_status | (ICUT_MAX << (ilp_port_num % 2) * 4);
        }
        if (dl_write_i2c(i2c_addr, ILP_POLICE_43_CONFIG, 1,
			 &wr_data) == FAILED) {
            return (FAILED);
        }
    }

    return (PASSED);
}

/* ******************************************************************
 *
 * Function: poe_2x_mode
 *
 * Description: Enable or Disable PoE 2x mode (support 802.3at, 
 *              30W power) in Dreamliner.
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
static int 
poe_2x_mode ()
{
    int port_num;
    int st_port, end_port;
    uchar user_input;
    int max_ports = get_port_num();

    if (is_daughter_card_present() == FALSE) {
        printf("The POE daughter card is not installed\n");
        return (PASSED);
    }

#if POE_DEBUG
    if ((check_poe_psu_present(POE_PSU_ONE, QUICK_MODE) == FALSE) && 
        (check_poe_psu_present(POE_PSU_TWO, QUICK_MODE) == FALSE)) {
#else
    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
#endif
//    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
        printf("\n WARNING, WARNING, Skipping POE 2X mode configuration.\n"
               " ILP power supply may not installed.\n");
        printf(" If ILP power supply is present then there is an ISSUE.\n"
               " Please check the power supply and ILP modules.\n");
        return (PASSED);
    }

    printf("This utility is to enable/disable PoE 2X mode.\n");

    port_num = getdec_answer("Enter port number(0 for all the ports): ", 
			     1, 0, get_port_num());
    st_port = 0;
    end_port = max_ports + st_port;

    user_input = getc_answer("To enable (e) or disable (d)? (q) to exit", 
                             "edq", 'e');
    if (user_input == 'q') {
         printf("exit\n");
         return (PASSED);
    }

    if(port_num != 0) {
	if (user_input == 'e') { /* enable */
	    if (poe_2x_mode_enable(port_num-1, 1) == FAILED) {
                return (FAILED);
            } else {
                printf("2x mode enable port%d pass\n", port_num);
            }
	} else {
	    if (poe_2x_mode_enable(port_num-1, 0) == FAILED) {
                return (FAILED);
            } else {
                printf("2x mode disable port%d pass\n", port_num);
            }
	}
    } else {
	/* to configure all the ports */
	for (port_num = st_port; port_num < end_port; port_num++) {
	    if (user_input == 'e') { /* enable */
		if (poe_2x_mode_enable(port_num, 1) == FAILED) {
		    return (FAILED);
		} else {
                    printf("2x mode enable port%d pass\n", port_num);
                }
	    } else {      
		if (poe_2x_mode_enable(port_num, 0) == FAILED) {
		    return (FAILED);
		} else {
                    printf("2x mode disable port%d pass\n", port_num);
                }
	    }
	}
    }

    return (PASSED);
}
/* ******************************************************************
 *
 * Function: poe_si_flag_write
 *
 * Description: Write the flag value to poe_flag.txt
 * 
 * Input: value
 * Outputs:  None
 *
 * ******************************************************************
 */

void 
poe_si_flag_write (char *buf)
{
    FILE *fp;

    /* remove file anyway */
    remove("poe_flag.txt");

    fp = fopen("poe_flag.txt", "w");
    if (fp == NULL) {
        printf("Failed to open poe_flag.txt file.\n");
	printf("Failed to open poe_flag.txt.\n");
    }
    fprintf(fp,"%s\n",buf);
    fclose(fp);
}
/* ******************************************************************
 *
 * Function: poe_si_flag_read
 *
 * Description: Read the flag value from poe_flag.txt
 * 
 * Input: None
 * Outputs:  TRUE / FALSE
 *
 * ******************************************************************
 */
boolean  
poe_si_flag_read (void)
{
    FILE *fp;
    char buf[10];
    boolean value;
    fp = fopen("poe_flag.txt", "r");
    if (fp == NULL) {
        printf("Failed to open poe_flag.txt file.\n");
    }
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
            if (strstr(buf, "TRUE") != NULL) {
                value = TRUE;
                break;
            }
            if (strstr(buf, "FALSE") != NULL) {
                value = FALSE;
                break;
            }
    }
    fclose(fp);
    return (value);
}
/* ******************************************************************
 *
 * Function: identical_poe_chip_check
 *
 * Description: Check if two PoE controllers on 8 port 
 *              Dreamliner are identical.
 *              Avoid two different parts on one Dreamliner PoE module.
 * 
 * Input:
 *
 * Outputs:  PASSED - Two chips are identical.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
static int 
identical_poe_chip_check ()
{
    uchar i2c_addr0 = POE_I2C_ADDR0, device_id0 = 0, firmware_rev0 = 0;
    uchar i2c_addr1 = POE_I2C_ADDR1, device_id1 = 0, firmware_rev1 = 0;
    uchar i2c_addr2 = POE_I2C_ADDR2;
    poe_si_flag_write("FALSE");
        
    /* Read Device ID of both chips */
    if (dl_read_i2c(i2c_addr0, ILP_ID, 1, &device_id0) == FAILED) {
        return (FAILED);
    }    
    if ((device_id0 & 0xf0) == TI_MSR_ID) {
        poe_si_flag_write("FALSE");
        printf("\nPoE chip is TI, Device ID = %x\n", device_id0);
    } else if ((device_id0 & 0xf0) == SI_MSR_ID) {
        poe_si_flag_write("TRUE");
        printf("\nPoE chip is SI, Device ID = %x\n", device_id0);
    }         
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nI2C addr 0x%x Device ID 0x%x\n", i2c_addr0, device_id0);
        fflush(0);
    }
    if (poe_si_flag_read() == TRUE) {
        if (dl_read_i2c(i2c_addr2, ILP_ID, 1, &device_id1) == FAILED) { /* Si3457  SI PoE */
            return (FAILED);
        }   
        i2c_addr1 = i2c_addr2;
    } else {
        if (dl_read_i2c(i2c_addr1, ILP_ID, 1, &device_id1) == FAILED) { /* TPS2386 IT PoE */
        return (FAILED);
    }   
    }   

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("I2C addr 0x%x Device ID 0x%x\n", i2c_addr1, device_id1);
        fflush(0);
    }
    /* Compare the Device ID */
    if (device_id0 != device_id1) {
        printf("Device ID are not identical. Device_ID0 = 0x%x & "
               "Device ID1 = 0x%x\n", device_id0, device_id1);
        fflush(0);
        return (FAILED);
    }

    /* Read Firmware revision of both chips */
    if (dl_read_i2c(i2c_addr0, ILP_FIRMWARE_REV, 1, &firmware_rev0) == FAILED) {
        return (FAILED);
    }    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("I2C addr 0x%x Firmware Rev 0x%x\n", i2c_addr0, firmware_rev0);
        fflush(0);
    }

    if (poe_si_flag_read() == TRUE) {
        if (dl_read_i2c(i2c_addr2, ILP_FIRMWARE_REV, 1, &firmware_rev1) == FAILED) { /* Si3457  SI PoE */
            return (FAILED);
        }   
        i2c_addr1 = i2c_addr2;
    } else {
        if (dl_read_i2c(i2c_addr1, ILP_FIRMWARE_REV, 1, &firmware_rev1) == FAILED) { /* TPS2386 IT PoE */
        return (FAILED);
    }   
    }   
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("I2C addr 0x%x Firmware Rev 0x%x\n", i2c_addr1, firmware_rev0);
        fflush(0);
    }

    /* Compare the Firmware revision */
    if (firmware_rev0 != firmware_rev1) {
        printf("Firmware Rev are not identical. Firmware Rev0 = 0x%x & "
               "Firmware Rev1 = 0x%x\n", firmware_rev0, firmware_rev1);
        fflush(0);
        return (FAILED);
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: poe_i2c_register_tests
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : reg_ptr - info for all registers
 *         i2c_addr
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
static int 
poe_i2c_register_tests (const reg_info_t *reg_ptr, uint i2c_addr)
{
    uint  ix, retval, tst_offset;
    uchar temp, readval, data, save_val, ret_val = PASSED;

    readval = 0;

    while (reg_ptr->size.size != 0) {
        retval = dl_read_i2c(i2c_addr, reg_ptr->offset, 
			     reg_ptr->size.size, &save_val);
        if (retval == FAILED) {
            cterr('f', 0, "Error reading %s register offset %#x, i2c addr "
                          "%#x\n", reg_ptr->name, reg_ptr->offset, i2c_addr);
            return (FAILED);
        }

        /*
         * Test a register if it's a R/W register
         */
        if (reg_ptr->type == READ_WRITE) {
            tst_offset = reg_ptr->offset;
            
            /*
             * ripple 1 test
             */
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp)
                    continue;    
                /* Write to register under test */
                retval = dl_write_i2c(i2c_addr, tst_offset, 
				      reg_ptr->size.size, &temp);
                /* Read back */
                if (retval == PASSED) {
                    ret_val = dl_read_i2c(i2c_addr, tst_offset, 
					  reg_ptr->size.size, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    cterr('f', 0, "Ripple one test failed "
                                  "when accessing %s Register offset %#x\n"
                                  "i2c addr %#x, Expect %#x, Read %#x ",
                                   reg_ptr->name, tst_offset,
                                   i2c_addr, temp, readval);
                    return (FAILED);
                }
            }
            msleep(50); /* 50 msec delay after the register wriet */

            /*
             * ripple 0 test
             */
            for (ix = 0; ix < 8; ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp)
                    continue;
                temp = (~(1 << ix)) & reg_ptr->mask;
                /* Write to register under test */
                retval = dl_write_i2c(i2c_addr, tst_offset, 
				      reg_ptr->size.size, &temp);
                /* Read back */
                if (retval == PASSED) {
                    ret_val = dl_read_i2c(i2c_addr, tst_offset, 
					  reg_ptr->size.size, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    cterr('f', 0, "Ripple zero test failed "
                                  "when accessing %s Register offset %#x\n"
                                  "i2c addr %#x, Expect %#x, Read %#x ",
                                   reg_ptr->name, tst_offset,
                                   i2c_addr, temp, readval);
                    return (FAILED);
                }
            }

            /*
             * pattern test
             */
            data = (uchar)PATTERN;
            for (ix=0; ix<2; ix++) {
                temp = data & reg_ptr->mask;
                retval = dl_write_i2c(i2c_addr, tst_offset, 
				      reg_ptr->size.size, &temp);
                if (retval == PASSED) {
                    ret_val = dl_read_i2c(i2c_addr, tst_offset, 
					  reg_ptr->size.size, &readval);
                }
                if ((readval & reg_ptr->mask) != temp) {
                    cterr('f', 0, "Pattern test failed when accessing %s "
                                  "Register offset %#x\n"
                                  "i2c addr %#x, Expect: %#x, Read: %#x",
                                   reg_ptr->name, tst_offset,
                                   i2c_addr, temp, readval);
                    return (FAIL);
                }
                data = (uchar)~PATTERN; /* complement data pattern */
            }

            /*
             * restore reset value
             */
            if (dl_write_i2c(i2c_addr, tst_offset, 
			     reg_ptr->size.size, &save_val) 
                == FAILED) {
		cterr('f', 0, "Failed to restore the original value for "
		      "Register offset %#x", tst_offset);
                return (FAILED);
            }
        }
        reg_ptr++;
    }
    return (PASSED);
}


/* ******************************************************
 *
 * Function: poe_reg_test
 *
 * Description: Tests PoE controller registers.
 *              I2C ruuning @100KHz speed (default)
 *
 * Input:   
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
int 
poe_reg_test ()
{
    prpass(testpass, "ILP Register,");
    cterr_setup();

    if (is_daughter_card_present() == FALSE) {
        printf("The POE daughter card is not installed\n");
        return (PASSED);
    }

#if POE_DEBUG
    if ((check_poe_psu_present(POE_PSU_ONE, QUICK_MODE) == FALSE) && 
        (check_poe_psu_present(POE_PSU_TWO, QUICK_MODE) == FALSE)) {
#else
    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
#endif
        printf("\n WARNING: ILP power supply may not be installed.\n");
        printf(" If ILP power supply is present then there is an ISSUE.\n"
               " Please check the power supply and ILP modules.\n");
        return (PASSED);
    }

    cterr_add_component("POE controller 0", 
			"I2C controller with the FPGA");
    cterr_add_debug("Check POE controller 0",
		    "Check I2C controller with the FPGA");
    if (poe_i2c_register_tests(poe_reg_test_tbl, POE_I2C_ADDR0) == FAILED) {
        return (FAILED);
    }

    cterr_add_component("POE controller 0", 
			"POE controller 1");
    cterr_add_debug("Check POE controller 0",
		    "Check POE controller 1");

    if (identical_poe_chip_check() == FAILED) {
	cterr('f', 0, "Two POE controller chips are not the same part.\n");
	return (FAILED);
    }

    cterr_add_component("POE controller 1", 
			"I2C controller with the FPGA");
    cterr_add_debug("Check POE controller 1",
		    "Check I2C controller with the FPGA");
    if (poe_si_flag_read() == TRUE) {
        if (poe_i2c_register_tests(poe_reg_test_tbl, POE_I2C_ADDR2) == FAILED) {
            return (FAILED);
        }
    } else {
    if (poe_i2c_register_tests(poe_reg_test_tbl, POE_I2C_ADDR1) == FAILED) {
	return (FAILED);
    }
    }   
    return (PASSED);
}

/* ******************************************************************
 *
 * Function: dl_power_detect
 *
 * Description: Check for Power Devices (PD's) at all ports in Dreamliner.
 *
 * Input:    
 *
 * Outputs:  PASSED.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
static int 
dl_power_detect ()
{
    uint max_port, ext_port, phy_port;

    max_port = get_port_num();
    
    for (ext_port = 1; ext_port <= max_port; ext_port++ ) {
	if (ext_port % 2)
	    phy_port = ext_port;
	else
	    phy_port = ext_port - 2;
	    
        if (phy_detect_phone(phy_port) == PASSED) {
            printf(" DTE needs power on port %d.\n", ext_port);
        } else {
            printf(" DTE does not need power on port %d.\n", ext_port);
        }
    }

    return (PASSED);
}


/* ******************************************************************
 *
 * Function: poe_display_env
 *
 * Description: Display voltage and cunrent for each POE port.
 *
 * Input:    
 *
 * Outputs:  PASSED.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
static int 
poe_display_env ()
{
    uint32_t value1, value2;
    uint16_t val1, val2;
    uint8_t data1, data2;
    uint max_port, i, m;
    uchar i2c_addr;

    max_port = get_port_num();

    if (is_daughter_card_present() == FALSE) {
        printf("The POE daughter card is not installed\n");
        return (PASSED);
    }

#if POE_DEBUG
    if ((check_poe_psu_present(POE_PSU_ONE, QUICK_MODE) == FALSE) && 
        (check_poe_psu_present(POE_PSU_TWO, QUICK_MODE) == FALSE)) {
#else
    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
#endif
//    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
        printf("\n WARNING: ILP power supply may not be installed.\n");
        printf(" If ILP power supply is present then there is an ISSUE.\n"
               " Please check the power supply and ILP modules.\n");
        return (PASSED);
    }
    
    for (i = 0; i < max_port; i++ ) {
	if (i < DREAMLINER_4GE_PHY_PORTS) {
	    i2c_addr = POE_I2C_ADDR0;
	    m = i;
	} else {
        if (poe_si_flag_read() == TRUE) {
	        i2c_addr = POE_I2C_ADDR2;
        } else {
	    i2c_addr = POE_I2C_ADDR1;
        }   
	    m = i - 4;
	}
	/* read the current */
	if (dl_read_i2c(i2c_addr, ILP_IDC_P1_LSB+m*4, 1, &data1) == FAILED) {
	    return (FAILED);
	} 
	if (dl_read_i2c(i2c_addr, ILP_IDC_P1_MSB+m*4, 1, &data2) == FAILED) {
	    return (FAILED);
	} 
	val1 = ((data2 << 8) | data1) & 0x3fff;
	value1 = val1 * 12207 / 200;
	printf("port %d:  current = %duA\t", (i+1), value1);

	/* read the voltage */
	if (dl_read_i2c(i2c_addr, ILP_VDC_P1_LSB+m*4, 1, &data1) == FAILED) {
	    return (FAILED);
	} 
	if (dl_read_i2c(i2c_addr, ILP_VDC_P1_MSB+m*4, 1, &data2) == FAILED) {
	    return (FAILED);
	} 
	val2 = ((data2 << 8) | data1) & 0x3fff;
	value2 = val2 * 1831 / 500;
	printf("voltage = %dmV\t", value2);

	value2 = (val1 * val2)/100000 * 22351;
	printf("power = %2d.%02dW\n", (value2/1000000), (value2%1000000));
    }

    return (PASSED);
}


/* ******************************************************************
 *
 * Function: print_poe_port_status
 *
 * Description: Display port status as indicated by the POE controller
 *
 * Input:    data - POE port status register content.
 *
 * Outputs:  None.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
static void 
print_poe_port_status (uchar data)
{
    printf("    Class   = ");
    switch ((data & CLASS_MASK) >> 4) {
    case UNKNOWN_CLASS:
        printf("class status unknown    ");
        break;
    case CLASS1:
        printf("class 1         ");
        break;
    case CLASS2:
        printf("class 2         ");
        break;
    case CLASS3:
        printf("class 3         ");
        break;
    case CLASS4:
        printf("class 4         ");
        break;
    case CLASS0:
        printf("class 0         ");
        break;
    case OVERCURRENT:
        printf("overcurrent     ");
        break;
    default:
        printf("undefined - read as 0   ");
        break;
    }

    printf("Detect  = ");
    switch (data & DETECT_MASK) {
    case UNKNOWN_DETECT:
        printf("detect status unknown");
        break;
    case SHORT_CIRCUIT:
        printf("short circuit (<1V)");
        break;
    case RLOW:
        printf("RLOW (<15K)");
        break;
    case DETECT_GOOD:
        printf("detect good (15K < R < 33K)");
        break;
    case RHIGH:
        printf("RHIGH (>33K)");
        break;
    case OPEN_CIRCUIT:
        printf("open circuit");
        break;
    case MOSFET_FAULT:
        printf("MOSFET fault");
        break;
    default:
        printf("reserved");
        break;
    }
    printf("\n");
}


/* ******************************************************************
 *
 * Function: print_poe_mode
 *
 * Description: Display the operating mode as indicated by the POE controller
 *
 * Input: data - ILP operating mode register content.
 *
 * Outputs:  None.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
static void 
print_poe_mode (uchar mode, int new_line)
{
    switch (mode) {
    case SHUTDOWN:
        printf("shutdown");
        break;
    case MANUAL:
        printf("manual");
        break;
    case SEMIAUTO:
        printf("semiauto");
        break;
    case MODE_AUTO:
        printf("auto");
        break;
    default:
        printf("Invalid mode %#x", mode);
        break;
    }

    if (new_line) {
        printf("\n");
    }
}


/* ******************************************************************
 *
 * Function: clear_poe_events
 *
 * Description: Clear a given poe controller's events.
 *
 * Input: i2c_addr - I2C address for POE.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
static int 
clear_poe_events (uchar i2c_addr)
{
    uchar events = 0;

    /* Read to clear the POE events */
    if (dl_read_i2c(i2c_addr, ILP_POWER_EVENT_COR, 1, &events)
            == FAILED) {
        return (FAILED);
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("POE %#x Power Event %#x\n", i2c_addr, events);
    }
    
    if (dl_read_i2c(i2c_addr, ILP_DETECT_EVENT_COR, 1, &events)
            == FAILED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("POE %#x Detect Event %#x\n", i2c_addr, events);
    }
    
    if (dl_read_i2c(i2c_addr, ILP_FAULT_EVENT_COR, 1, &events)
            == FAILED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("POE %#x Fault Event %#x\n", i2c_addr, events);
    }
    
    if (dl_read_i2c(i2c_addr, ILP_TSTART_EVENT_COR, 1, &events)
            == FAILED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("POE %#x tSTART Event %#x\n", i2c_addr, events);
    }
    
    if (dl_read_i2c(i2c_addr, ILP_SUPPLY_EVENT_COR, 1, &events)
            == FAILED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("POE %#x Supply Event %#x\n", i2c_addr, events);
    }
    
    if (dl_read_i2c(i2c_addr, ILP_PWR_ON_FAULT_COR, 1, &events)
            == FAILED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("POE %#x Power-on Fault %#x\n", i2c_addr, events);
    }
 
    return (PASSED);
}

/* ******************************************************************
 *
 * Function: poe_power_ports
 *
 * Description: Power on or off all ports of a given POE.
 *
 * Input: 
 *        i2c_addr - I2C address for POE controller.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
static int 
poe_power_ports (uchar i2c_addr)
{
    uchar op_mode = 0, port_opmode = 0, power_status = 0, new_power_status = 0;
    uchar port_status = 0, detect_event = 0 , wr_data;
    uint poe_port, max_ports, ext_start_port, cisco_phone, user_input;
    static uchar phone_type[PORTS_PER_ILP];
    uchar curr_lsb = 0, curr_msb = 0, volt_lsb = 0, volt_msb = 0;
    uint power;
    uint led_port;

    if (i2c_addr == POE_I2C_ADDR0) {
        max_ports = POE_PORTS;
        ext_start_port = POE0_EXT_START_PORT; /* I/0 port starts from 0 */
    } else {
        max_ports = POE_PORTS;
        ext_start_port = POE1_EXT_START_PORT; /* I/0 port starts from 4 */
    }

    /* Clear pending POE events */
    if (clear_poe_events(i2c_addr) == FAILED) {
        return (FAILED);
    }

    /* Check for the operating mode for each ports. */
    if (dl_read_i2c(i2c_addr, ILP_OPERATING_MODE, 1, &op_mode) == FAILED) {
        return (FAILED);
    }

    for (poe_port = 0; poe_port < max_ports; poe_port++) {
        printf("\nPOE port %d: Mode = ", ext_start_port + poe_port + 1);
        print_poe_mode((op_mode >> (poe_port * 2)) & MODE_MASK, FALSE);
        if (((op_mode >> (poe_port * 2)) & MODE_MASK) == SHUTDOWN) {
            /* Shutdown mode cannot power on/off the port. Change the mode */
            printf("\n To power on or off the port, ");
            printf(" the operating mode cannot be in the shutdown mode.\n");
            printf("The operating mode code: ");
            printf(" %d for Manual. %d for Semi-auto. %d for Auto\n",
                   MANUAL, SEMIAUTO, MODE_AUTO);
            /* Shutdown mode is 00, so clearing is not needed. Otherwise;*/
            port_opmode = getdec_answer("Enter the new operating mode:",
                                        SEMIAUTO, MANUAL, MODE_AUTO) 
                                        & MODE_MASK;
            op_mode |= (port_opmode << (poe_port * 2));
            /* update the new mode */
            if (dl_write_i2c(i2c_addr, ILP_OPERATING_MODE, 1, &op_mode) == FAILED) {
                return (FAILED);
            }
        }
    }

    /* Enable Disconnect */
    wr_data = DC_DIS_ALL;
    if (dl_write_i2c(i2c_addr, ILP_DISCONNECT_ENABLE, 1, &wr_data) == FAILED) {
        return (FAILED);
    }

    printf("\n\nWARNING: Perform PoE power on with loopback cable(s) "
           "may damage the PHY component!! \n"  
           "         Please make sure you have correct PD(s).\n");

    user_input = getc_answer("To power on (i) or off (o)? (q) to exit", 
                             "ioq", 'i');
    if (user_input == 'q') {
        return (PASSED);
    }

    if (user_input == 'i') {
        /* Turn the power on */
        for (poe_port = 0; poe_port < max_ports; poe_port++) {
            cisco_phone = FALSE;    /* Assume 802.3af */
	    /* poe_port   led_port 
	       0          1
	       1          0
	       2          3
	       3          2
	       4          5
	       5          4
	       6          7
	       7          6
	    */
	    if (poe_port % 2)
		led_port = ext_start_port + poe_port - 1;
	    else 
		led_port = ext_start_port + poe_port + 1;

            /* Get the operating mode of the port */
            port_opmode = (op_mode >> (poe_port * 2)) & MODE_MASK;
            if (dl_read_i2c(i2c_addr, ILP_POWER_STATUS, 1, 
			    &power_status) == FAILED) {
                return (FAILED);
            }
            power_status &= POWER_GOOD_MASK;
            printf("\nOriginal power status: Port %d powered %3s. \n", 
                    ext_start_port + poe_port + 1,
            (power_status & (POWER_GOOD_1 << poe_port)) ? "On" : "Off");
            if (power_status & (POWER_GOOD_1 << poe_port)) {
                /* Already powered on */
                printf("Power already on. ");
                printf("Reading the Class/Detect set earlier\n");
            } else {
                /* Was powered off */
                phone_type[poe_port]= NO_PHONE;   /* Init as no phone */
                /* Enable Class and Detect */
                printf("Powering on port %d ...\n", ext_start_port + poe_port + 1);
                wr_data = ((CLASS_ENABLE_1 + DETECT_ENABLE_1) << poe_port);
                if (dl_write_i2c(i2c_addr, ILP_DET_CLASS_RESTART_PB,
				 1, &wr_data) == FAILED) {
                    return (FAILED);
                }
                msleep(600);    /* 460 ms Detect time tDETECT */

                /* Check for detect complete */
                if (dl_read_i2c(i2c_addr, ILP_DETECT_EVENT, 1,
                                     &detect_event) == FAILED) {
                    return (FAILED);
                }
                if (detect_event & (DETECT_COMPLETE_1 << poe_port)) {
                    /* Detect complete */
                    /* Wait for Class to complete */
                    switch (port_opmode) {
                    case MANUAL:
                        /* Manual mode */
                        msleep(300);    /* 265 ms tCLASS */
                        break;
                    default:
                        /* Semi-auto and Auto mode */
                        printf("Please wait for 5 seconds.");
			/* 47 ms tCLASS But Cisco IP phone needs more time */
                        msleep(5000);   
                        break;
                    }

                    /* Check for class complete */
                    if (dl_read_i2c(i2c_addr, ILP_DETECT_EVENT, 1,
                                         &detect_event) == FAILED) {
                        return (FAILED);
                    }
                    if (detect_event & (CLASS_COMPLETE_1 << poe_port)) {
                        /* Class complete */
                        phone_type[poe_port] = IEEE_PHONE;
                    } else {
                        /* Unable to get the class */
                        /* Check for short circuit */
                        if (dl_read_i2c(i2c_addr, ILP_PORT1_STATUS + poe_port,
					1, &port_status) == FAILED) {
                            return (FAILED);
                        }
                        if ((port_status & DETECT_MASK) == SHORT_CIRCUIT) {
                            printf("\rWarning: Short circuit detected.\n"
                                   "Please make sure no loopback cable and "
                                   "check the circuit.\n");
                            phone_type[poe_port] = NO_PHONE;
                            return (FAILED);
                        }

                        /* Check for Cisco PD */
                        if (phy_detect_phone(ext_start_port + poe_port) == PASSED) {
                            printf("\rCisco PD detected\n");
                            cisco_phone = TRUE;
                            phone_type[poe_port] = CISCO_PHONE;
                        } else {
                            /* Not Cisco phone */
                            printf("Unable to read the class\n");
                            phone_type[poe_port] = NO_PHONE;
                        }
                    }
                } else {
                    /* Unable to detect */
                    /* Check for short circuit */
                    if (dl_read_i2c(i2c_addr, ILP_PORT1_STATUS + poe_port,
				    1, &port_status) == FAILED) {
                        return (FAILED);
                    }
                    if ((port_status & DETECT_MASK)== SHORT_CIRCUIT) {
                        printf("\rWarning: Short circuit detected.\n"
                               "Please make sure no loopback cable and "
                               "check the circuit.\n");
                        phone_type[poe_port] = NO_PHONE;
                        return (FAILED);
                    }

                    /* Check for Cisco PD */
                    if (phy_detect_phone(ext_start_port + poe_port) == PASSED) {
                        printf("\rCisco PD detected\n");
                        cisco_phone = TRUE;
                        phone_type[poe_port] = CISCO_PHONE;
                    } else {
                        /* Not Cisco phone */
                        phone_type[poe_port] = NO_PHONE;
                        if (port_opmode == MANUAL) {
                            /* For manual mode, Detect does not have to
                             * complete */
                            printf("Detect not complete in manual mode\n");
                        } else {
                            /* For Semi-auto or Auto mode, Detect has to
                             * complete */
                            printf("Unable to detect\n");
                        } /* endof if port_opmode */
                    } /* endof if phy_detect_phone */
                } /* endof if (detect_event) */

                /* If Cisco PD, check for the mode */
                if ((cisco_phone == TRUE) && (port_opmode == MODE_AUTO)) {
                    /* Cisco phone in auto mode, change it to semi-auto mode */
                    printf("Cisco PD cannot operate in Auto mode.");
                    printf(" Switch to Semiauto mode\n");
                    /* clear the mode bit */
                    op_mode &= (~(MODE_MASK << (poe_port * 2)));

                    port_opmode = SEMIAUTO;
                    op_mode |= (port_opmode << (poe_port * 2));
                    if (dl_write_i2c(i2c_addr, ILP_OPERATING_MODE, 
				     1, &op_mode) == FAILED) {
                       return (FAILED);
                    }
                    msleep(10);
                }
                /* Power on the port. If in auto mode, and if class complete,
                 * power on is automatic */
                if ((port_opmode != MODE_AUTO)
                    || (detect_event & (CLASS_COMPLETE_1 << poe_port))) {
                    /* Power on the port */
                    wr_data = POWER_ON_1 << poe_port;
                    if (dl_write_i2c(i2c_addr, ILP_POWER_ENABLE_PB, 
				     1, &wr_data) == FAILED) {
                        return (FAILED);
                    }
                }
            } /* endof if (power_status) */
            if (cisco_phone == FALSE) {
                /* Read Class from Port status register */
                if (dl_read_i2c(i2c_addr, poe_port + ILP_PORT1_STATUS, 1, &port_status) 
		    == FAILED) {
                    return (FAILED);
                }
                if (port_opmode == MANUAL) {
                    printf("Manual mode. ");
                    printf("Class/Detect shown may not be accurate\n");
                }
                print_poe_port_status(port_status);
            }

            /* setup expected power status */
            power_status |= (POWER_GOOD_1 << poe_port);

            /* wait for the power good status to settle tDIS - 720 ms max */
            msleep(800);
            if (dl_read_i2c(i2c_addr, ILP_POWER_STATUS, 1, 
			    &new_power_status) == FAILED) {
                return (FAILED);
            }

            if ((power_status & (POWER_GOOD_1 << poe_port)) != 
                (new_power_status & POWER_GOOD_MASK & (POWER_GOOD_1 << poe_port))) {
                /* Not the expected power status. turn on yellow LED */
		if (led_class_config(0, FALSE, FALSE, 0, TRUE, 1 << led_port)) {
		    cterr('f',0,"Failed to configure LED class 0");
		    return FAILED;
		}

		if (led_class_config(1, FALSE, FALSE, 0, TRUE, 0)) {
		    cterr('f',0,"Failed to configure LED class 1");
		    return FAILED;
		}

                printf("Expect power status %#x, received %#x\n",
                power_status, new_power_status & POWER_GOOD_MASK);
            } else {
                /* power state verified */
                if (power_status & (POWER_GOOD_1 << poe_port)) {
                    /* Power on. Turn on green LED */
		    if (led_class_config(0, FALSE, FALSE, 0, TRUE, 0)) {
			cterr('f',0,"Failed to configure LED class 0");
			return FAILED;
		    }
	
		    if (led_class_config(1, FALSE, FALSE, 0, TRUE, 1 << led_port)) {
			cterr('f',0,"Failed to configure LED class 1");
			return FAILED;
		    }
                } else {
                    /* Power off. Turn off LED */
		    if (led_class_config(0, FALSE, FALSE, 0, TRUE, 0)) {
			cterr('f',0,"Failed to configure LED class 0");
			return FAILED;
		    }
		    if (led_class_config(1, FALSE, FALSE, 0, TRUE, 0)) {
			cterr('f',0,"Failed to configure LED class 1");
			return FAILED;
		    }
                }
            } /* endof if (power_status) */
        } /* endof for */
        msleep(5000); /* 5 sec delay before display power */
    } else {
        /* Power off */
        for (poe_port = 0; poe_port < max_ports; poe_port++) {
            phone_type[poe_port]= NO_PHONE;
            printf("Powering off port %d ...\n", ext_start_port + poe_port + 1);

            /* setup expected power status */
            power_status &= ~(POWER_GOOD_1 << poe_port);
            wr_data = POWER_OFF_1 << poe_port;
            if (dl_write_i2c(i2c_addr, ILP_POWER_ENABLE_PB, 1,
			     &wr_data) == FAILED) {
                return (FAILED);
            }

            /* Power off. Turn off LED */
	    if (led_class_config(0, FALSE, FALSE, 0, TRUE, 0)) {
		cterr('f',0,"Failed to configure LED class 0");
		return FAILED;
	    }

	    if (led_class_config(1, FALSE, FALSE, 0, TRUE, 0)) {
		cterr('f',0,"Failed to configure LED class 1");
		return FAILED;
	    }
        }
    }

    /* Summarize the port status*/
    printf("\n\nPort\tMode\t\tOn/Off\tPhone\tClass\tPower(Watt)\t\n");
    for (poe_port = 0; poe_port < max_ports; poe_port++) {
        /* Print port num */
        printf("%d\t", ext_start_port + poe_port + 1);

        /* Print Port Mode*/
        if (dl_read_i2c(i2c_addr, ILP_OPERATING_MODE, 1, 
                             &op_mode) == FAILED) {
            return (FAILED);
        }
        switch ((op_mode >> (poe_port * 2)) & MODE_MASK) {
        case SHUTDOWN:
            printf("shutdown\t");
            break;
        case MANUAL:
            printf("manual\t");
            break;
        case SEMIAUTO:
            printf("semiauto\t");
            break;
        case MODE_AUTO:
            printf("auto\t");
            break;
        default:
            printf("unknown\t ");
            break;
        }

        /* Print Power Status On/Off */
        if (dl_read_i2c(i2c_addr, ILP_POWER_STATUS, 1, 
                             &power_status) == FAILED) {
            return (FAILED);
        }
        power_status &= POWER_GOOD_MASK;
        printf("%3s\t", (power_status & (POWER_GOOD_1 << poe_port)) ? 
               "On" : "Off");

        /* Print Phone Type */
        switch (phone_type[poe_port]) {
        case NO_PHONE:
            printf("None\n");
            continue;
        case CISCO_PHONE:
            printf("Cisco\t");
            break;
        case IEEE_PHONE:
            printf("IEEE\t");
            break;
        default:
            printf("unknown\n");
            continue;
        }

        /* Print Class Status */
        if (dl_read_i2c(i2c_addr, ILP_PORT1_STATUS + poe_port, 1, 
                             &port_status) == FAILED) {
            return (FAILED);
        }
        switch ((port_status & CLASS_MASK) >> 4) {
        case UNKNOWN_CLASS:
            printf("unknown\t");
            break;
        case CLASS1:
            printf("1\t");
            break;
        case CLASS2:
            printf("2\t");
            break;
        case CLASS3:
            printf("3\t");
            break;
        case CLASS4:
            printf("4\t");
            break;
        case CLASS0:
            printf("0\t");
            break;
        case OVERCURRENT:
            printf("overcurrent\t");
            break;
        default:
            printf("\t");
            break;
        }

        /* Print Power*/
	if (dl_read_i2c(i2c_addr, ILP_IDC_P1_LSB + 4*poe_port, 1, &curr_lsb) == FAILED) {
	    return (FAILED);
	}
	if (dl_read_i2c(i2c_addr, ILP_IDC_P1_MSB + 4*poe_port, 1, &curr_msb) == FAILED) {
	    return (FAILED);
	}
	if (dl_read_i2c(i2c_addr, ILP_VDC_P1_LSB + 4*poe_port, 1, &volt_lsb) == FAILED) {
	    return (FAILED);
	}
	if (dl_read_i2c(i2c_addr, ILP_VDC_P1_MSB + 4*poe_port, 1, &volt_msb) == FAILED) {
	    return (FAILED);
	}

	power = ((uint)curr_lsb + ((uint)curr_msb << 8)) * 
	    ((uint)volt_lsb + ((uint)volt_msb << 8)) / 100000 * 22351;
	printf("%2d.%02dw\n", (power/1000000), (power%1000000));
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
    if (is_daughter_card_present() == FALSE) {
        printf("The POE daughter card is not installed\n");
        return (PASSED);
    }

#if POE_DEBUG
    if ((check_poe_psu_present(POE_PSU_ONE, QUICK_MODE) == FALSE) && 
        (check_poe_psu_present(POE_PSU_TWO, QUICK_MODE) == FALSE)) {
#else
    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
#endif
//    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
        printf("\n WARNING, WARNING, Skipping POE Power On/Off test.\n"
               " ILP power supply may not be installed.\n");
        printf(" If ILP power supply is present then there is an ISSUE.\n"
               " Please check the power supply and ILP modules.\n");
        return (PASSED);
    }

    if (poe_power_ports(POE_I2C_ADDR0) == FAILED) {
        return (FAILED);
    }

    if (poe_si_flag_read() == TRUE) {
        if (poe_power_ports(POE_I2C_ADDR2) == FAILED) {
            return (FAILED);
        }
    } else {
    if (poe_power_ports(POE_I2C_ADDR1) == FAILED) {
	return (FAILED);
    }
    }   

    return (PASSED);

}

/******************************************************************************
 *
 * Function   :	poe_init
 * Description:	mask off all the interrupts for both POE controllers.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
poe_init ()
{
    uint8_t data;
    int status;
    uchar device_id0 = 0;

    cterr_add_component("POE controller 0", 
			"I2C controller with the FPGA");
    cterr_add_debug("Check POE controller 0",
		    "Check I2C controller with the FPGA");

    data = 0;

    /* Read Device ID of both chips */
    if (dl_read_i2c(POE_I2C_ADDR0, ILP_ID, 1, &device_id0) == FAILED) {
        return (FAILED);
    }    
    if ((device_id0 & 0xf0) == TI_MSR_ID) {
        poe_si_flag_write("FALSE");
        printf("\nPoE chip is TI, Device ID = %x ", device_id0);
        fflush(0);
    } else if ((device_id0 & 0xf0) == SI_MSR_ID) {
        poe_si_flag_write("TRUE");
        printf("\nPoE chip is SI, Device ID = %x ", device_id0);
        fflush(0);
    } else {
        printf("\nPoE chip invalid , Device ID = %x ", device_id0);
    }         
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nI2C addr 0x%x Device ID 0x%x\n", POE_I2C_ADDR0, device_id0);
        fflush(0);
    }

    status = dl_write_i2c(POE_I2C_ADDR0, ILP_INT_MASK, 1, &data);
    if (status != PASSED) {
        cterr('f',0, "POE I2C write failed for POE0.");
        return (FAILED);
    }

    cterr_add_component("POE controller 1", 
			"I2C controller with the FPGA");
    cterr_add_debug("Check POE controller 1",
		    "Check I2C controller with the FPGA");

    if (poe_si_flag_read() == TRUE) {
        status = dl_write_i2c(POE_I2C_ADDR2, ILP_INT_MASK, 1, &data);
    } else {
        status = dl_write_i2c(POE_I2C_ADDR1, ILP_INT_MASK, 1, &data);
    }   
    if (status != PASSED) {
        cterr('f',0, "POE I2C write failed for POE1.");
        return (FAILED);
    }

    cterr_add_component("Dreamliner FPGA", 
			"SMI1 interface from xCat2 switch");
    cterr_add_debug("Dreamliner FPGA",
		    "SMI1 interface from xCat2 switch");

    /* clear any pending FPGA interrupts */
    status = smi1_write_reg(FPGA_INTR_STATUS, 0x7f);   
    if (status != PASSED) {
        cterr('f',0, "failed to clear FPGA interrupt.");
        return (FAILED);
    }  
    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	poe_intr_test
 * Description:	Verify POE is able to generate interrupt to the host via PonCat2 swtich.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
poe_intr_test ()
{
    uint8_t data; 
    uint16_t fpga_data;
    uint32_t rd_data;
    int status, i;
    uchar i2c_addr = POE_I2C_ADDR0;
    unsigned long config_base = dl_get_pci_base_addr();

    cterr_setup();
    if (is_daughter_card_present() == FALSE) {
        printf("The POE daughter card is not installed\n");
        return (PASSED);
    }

#if POE_DEBUG
    if ((check_poe_psu_present(POE_PSU_ONE, QUICK_MODE) == FALSE) && 
        (check_poe_psu_present(POE_PSU_TWO, QUICK_MODE) == FALSE)) {
#else
    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
#endif
        printf("\n WARNING: ILP power supply may not be installed.\n");
        printf(" If ILP power supply is present then there is an ISSUE.\n"
               " Please check the power supply and ILP modules.\n");
        return (PASSED);
    }

    prpass(testpass, "POE interrupt test, ");

    poe_intr_happened = FALSE;

    /* to be safe, init POE controller again. */
    if (poe_init() == FAILED) {
        cterr('f',0,"Failed to init POE controller.");	    
        return (FAILED);
    }

    cterr_add_component("POE controller 0", 
			"Dreamliner FPGA",
			"Marvell xCat2 switch");
    cterr_add_debug("Check POE controller 0",
		    "Check Dreamliner FPGA",
		    "Check Marvell xCat2 switch");

   /* before the test, make sure the GPP1 within PonCat2 is unasserted */
    status = xcat2_reg_pci_read(GPP_INPUT_REG_OFFSET, &rd_data);
    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("GPP_INPUT_REG_OFFSET @ %#x = %#x\n", GPP_INPUT_REG_OFFSET, rd_data);
    }
    if (status == PASSED) {
	if ((rd_data & GPP_FPGA_INT) != GPP_FPGA_INT) {
	    cterr('f',0,"Before the POE interrupt test, GPP1 pin is asserted.");
	    return (FAILED);
	}
    } else {
	cterr('f',0,"Failed to read POE intr status from PonCat2.");
	return (FAILED);
    }

    /* before the test, set to manual mode and force to turn on port 1 */
    data = 0x1;
    status = dl_write_i2c(i2c_addr, ILP_OPERATING_MODE, 1, &data);
    if (status != PASSED) {
        cterr('f',0,"Failed to set port 1 in manual mode.\n");
        return (FAILED);
    }
    data = 0x1;
    status = dl_write_i2c(i2c_addr, ILP_POWER_ENABLE_PB, 1, &data);
    if (status != PASSED) {
        cterr('f',0,"Failed to force turn on port 1.\n");
        return (FAILED);
    }
    msleep(200);

    /* enable POE power enable status change interrupt (register 1, bit 0) */
    data = POWER_ENABLE;
    status =  dl_write_i2c(i2c_addr, ILP_INT_MASK, 1, &data);
    if (status != PASSED) {
	cterr('f',0, "POE I2C write failed.");
        return (FAILED);
    }

    /* clear any pending FPGA interrupts */
    status = smi1_write_reg(FPGA_INTR_STATUS, 0x7f);   
    if (status != PASSED) {
	cterr('f',0, "failed to clear FPGA interrupt.");
        return (FAILED);
    }

    /* enable FPGA POE interrupt */
    status = smi1_write_reg(FPGA_INTR_MASK, ~FPGA_POE_INTR_BIT);   
    if (status != PASSED) {
	cterr('f',0, "failed to enable FPGA POE interrupt.");
        return (FAILED);
    }

    /* enable FPGA global interrupt */
    status = smi1_write_reg(FPGA_GLOBAL_INTR_MASK, 0x0);   
    if (status != PASSED) {
	cterr('f',0, "failed to enable FPGA global interrupt.");
        return (FAILED);
    }

    /* clear xcat2 GPIO interrupt status */
    *(unsigned int *)(config_base + GPIO_INTR_CAUSE_REG) = 0;
    if ((NVRAM)->diagflag & D_VERBOSE) {
	rd_data = *(unsigned int *)(config_base + 0x10114);
	printf("gpio_int_cause_reg = %#x\n", rd_data);
	rd_data = *(unsigned int *)(config_base + 0x10110);
	printf("gpio_data_in_reg = %#x\n", rd_data);
    }

    /* enable xcat2 GPIO interrupts for FPGA */
    *(unsigned int *)(config_base + GPIO_INTR_LEVEL_MASK_REG) = (1 << GPIO_FPGA_INT);

    /* force to turn off port 1 to generate power good status change interrupt */
    data = 0x10;
    status = dl_write_i2c(i2c_addr, ILP_POWER_ENABLE_PB, 1, &data);
    if (status != PASSED) {
        cterr('f',0,"Failed to force turn off port 1.\n");
        return (FAILED);
    }
    msleep(1000);

    /* read POE interrupt status register */
    dl_read_i2c(i2c_addr, ILP_INTERRUPT, 1, &data);
    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("\nPOE_INTERRUPT @ %#x = %#x\n", ILP_INTERRUPT, data);
    }
    if (!(data & POWER_ENABLE)) {
	cterr('f',0,"POE interrupt for power status change is not set. intr = %#x", data);
	status = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	dl_read_i2c(i2c_addr, ILP_INT_MASK, 1, &data);
	printf("\nPOE_INTR_MASK = %#x\n", data);
	status = xcat2_reg_pci_read(GPP_INPUT_REG_OFFSET, &rd_data);
	printf("GPP_INPUT_REG_OFFSET @ %#x = %#x\n", GPP_INPUT_REG_OFFSET, rd_data);
	/* read FPGA interrupt status register */
	smi1_read_reg(FPGA_INTR_MASK, &fpga_data);
	printf("FPGA_int_mask = %#x\n", fpga_data);
	smi1_read_reg(FPGA_GLOBAL_INTR_MASK, &fpga_data);
	printf("FPGA_global_int_mask = %#x\n", fpga_data);
    }

    smi1_read_reg(FPGA_INTR_STATUS, &fpga_data);
    if (!(fpga_data & FPGA_POE_INTR_BIT)) {
	cterr('f',0,"FPGA POE interrupt is not set. intr_status = %#x", fpga_data);
	status = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	rd_data = *(unsigned int *)(config_base + 0x20210);
	printf("main_int_cause_hi_reg = %#x\n", rd_data);
	rd_data = *(unsigned int *)(config_base + 0x10110);
	printf("gpio_data_in_reg = %#x\n", rd_data);
	rd_data = *(unsigned int *)(config_base + 0x10114);
	printf("gpio_int_cause_reg = %#x\n", rd_data);
    }

    for (i = 0; i < POE_INTR_DELAY; i++) {
	if (poe_intr_happened == TRUE) {
	    poe_intr_happened = FALSE;
	    break;
	}
	msleep(2);
    }

    if (i == POE_INTR_DELAY) {
	cterr('f', 0, "Timeout waiting for POE interrupt.");
	status = FAILED;
    }

    /* disable xcat2 GPIO interrupts */
    *(unsigned int *)(config_base + GPIO_INTR_LEVEL_MASK_REG) = 0;

    /* disable FPGA interrupts */
    smi1_write_reg(FPGA_GLOBAL_INTR_MASK, 0x1);
    smi1_write_reg(FPGA_INTR_MASK, 0x7f);  
    smi1_write_reg(FPGA_INTR_STATUS, 0x7f);  
 
    /* disable POE interrupts */
    data = 0;
    if (dl_write_i2c(i2c_addr, ILP_INT_MASK, 1, &data) != PASSED) {
	cterr('f',0, "POE I2C write failed.");
	status = FAILED;
    }

    /* set OFF mode for port 1 */
    data = 0;
    if (dl_write_i2c(i2c_addr, ILP_OPERATING_MODE, 1, &data) != PASSED) {
        cterr('f',0,"Failed to set port 1 in manual mode.\n");
        status = FAILED;
    }

    /* reset port 1 to clear all event registers */
    data = 0x1;
    if (dl_write_i2c(i2c_addr, ILP_GLOBAL_PB, 1, &data) != PASSED) {
	cterr('f',0, "POE I2C write failed.");
	status = FAILED;
    }

    return (status);
}

#ifdef YWEN
/* ******************************************************************
 *
 * Function: detect_ieee_phone
 *
 * Description: A while loop to detect IEEE phone for debug purpose.
 *              Only do detection and classification. No power on.
 *
 * Input:    iface - Pointer to interface data structure.
 *           i2c_addr 
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
static int detect_ieee_phone (hwic_iface_t *iface, uchar i2c_addr)
{
    uchar op_mode = 0, port_opmode = 0, power_status = 0, new_power_status = 0;
    uchar port_status = 0, detect_event = 0, wr_data;
    uint poe_port, max_ports, ext_start_port, cisco_phone;

    if (i2c_addr == POE_I2C_ADDR0) {
        max_ports = POE_PORTS;
        ext_start_port = POE0_EXT_START_PORT; /* I/0 port starts from 0 */
    } else {
        max_ports = POE_PORTS;
        ext_start_port = POE1_EXT_START_PORT; /* I/0 port starts from 4 */
    }

    while (1){
        /* Clear pending POE events */
        if (clear_sn2385_events(iface, i2c_addr) == FAILED) {
            return (FAILED);
        }
    
        /* Check for the operating mode for each ports. */
        if (dl_read_i2c(i2c_addr, ILP_OPERATING_MODE, 1, 
                             &op_mode) == FAILED) {
            return (FAILED);
        }
    
        for (poe_port = 0; poe_port < max_ports; poe_port++) {
            printf("\nILP port %d: Mode = ", poe_port);
            print_poe_mode((op_mode >> (poe_port * 2)) & MODE_MASK, FALSE);
            if (((op_mode >> (poe_port * 2)) & MODE_MASK) == SHUTDOWN) {
                /* Shutdown mode cannot power on/off the port. Change the mode*/
                printf("\n To power on or off the port, "
                       " the operating mode cannot be in the shutdown mode.\n"
                       "The operating mode code: "
                       " %d for Manual. %d for Semi-auto. %d for Auto\n",
                        MANUAL, SEMIAUTO, MODE_AUTO);
                /* Shutdown mode is 00, so clearing is not needed. Otherwise;*/
                port_opmode = getdec_answer("Enter the new operating mode:",
                                            SEMIAUTO, MANUAL, MODE_AUTO) 
                                            & MODE_MASK;
                op_mode |= (port_opmode << (poe_port * 2));
                /* update the new mode */
                if (firebee_write_i2c(iface, i2c_addr, ILP_OPERATING_MODE, 1, 
                                      &op_mode) == FAILED) {
                    return (FAILED);
                }
            }
        }
    
        /* Enable Disconnect */
        wr_data = AC_DIS_ALL;
        if (firebee_write_i2c(iface, i2c_addr, ILP_DISCONNECT_ENABLE, 1, 
                              &wr_data) == FAILED) {
            return (FAILED);
        }
    
        for (poe_port = 0; poe_port < max_ports; poe_port++) {
            cisco_phone = FALSE;    /* Assume 802.3af */
            /* Get the operating mode of the port */
            port_opmode = (op_mode >> (poe_port * 2)) & MODE_MASK;
            if (dl_read_i2c(i2c_addr, ILP_POWER_STATUS, 1, 
                                 &power_status) == FAILED) {
                return (FAILED);
            }
            power_status &= POWER_GOOD_MASK;
            printf("\n Port %d powered %3s. \n", poe_port,
                    (power_status & (POWER_GOOD_1 << poe_port)) ? "On" : "Off");
    
            if (power_status & (POWER_GOOD_1 << poe_port)) {
                /* Already powered on */
                printf("Power already on. ");
                printf("Reading the Class/Detect set earlier\n");
            } else {
                /* Was powered off */
                /* Enable Class and Detect */
                wr_data = ((CLASS_ENABLE_1 + DETECT_ENABLE_1) << poe_port);
                if (firebee_write_i2c(iface, i2c_addr, 
                                      ILP_DET_CLASS_RESTART_PB, 1, 
                                      &wr_data) == FAILED) {
                    return (FAILED);
                }
                msleep(600);    /* 460 ms Detect time tDETECT */

                /* Check for detect complete */
                if (dl_read_i2c(i2c_addr, ILP_DETECT_EVENT, 1,
                                     &detect_event) == FAILED) {
                    return (FAILED);
                }
                if (detect_event & (DETECT_COMPLETE_1 << poe_port)) {
                    /* Detect complete */
                    /* Wait for Class to complete */
                    switch (port_opmode) {
                    case MANUAL:
                        /* Manual mode */
                        msleep(300);    /* 265 ms tCLASS */
                        break;
                    default:
                        /* Semi-auto and Auto mode */
                        prpass(testpass, "Up to 5 seconds delay,");
                        msleep(5000);   /* 47 ms tCLASS But Cisco IP
                                         * phone needs more time */
                        break;
                    }

                    /* Check for class complete */
                    if (dl_read_i2c(i2c_addr, ILP_DETECT_EVENT,
                                         1, &detect_event) == FAILED) {
                        return (FAILED);
                    }
                    if (detect_event & (CLASS_COMPLETE_1 << poe_port)) {
                        /* Class complete */
                        printf("\rA IEEE phone is detected on port %d \n",
                                poe_port);
                    }
                } else {
                    /* Unable to detect */
                    printf("\rNo phone is detected on port %d \n", 
                            poe_port);
                } /* endof if (detect_event) */


            } /* endof if (power_status) */

            if (cisco_phone == FALSE) {
                /* Read Class from Port status register */
                if (dl_read_i2c(i2c_addr, 
                                     poe_port + ILP_PORT1_STATUS,
                                     1, &port_status) == FAILED) {
                    return (FAILED);
                }
                if (port_opmode == MANUAL) {
                    printf("Manual mode. ");
                    printf("Class/Detect shown may not be accurate\n");
                }
                print_firebee_port_status(port_status);
            }

            /* setup expected power status */
            power_status |= (POWER_GOOD_1 << poe_port);
    
            /* wait for the power good status to settle tDIS - 720 ms max */
            msleep(800);
            if (dl_read_i2c(i2c_addr, ILP_POWER_STATUS, 1, 
                                 &new_power_status) == FAILED) {
                return (FAILED);
            }
            if ((power_status & (POWER_GOOD_1 << poe_port)) != 
                (new_power_status & POWER_GOOD_MASK & 
                (POWER_GOOD_1 << poe_port))) {
                /* Not the expected power status. turn on yellow LED */
                firebee_led(iface, ext_start_port + poe_port, 
                            FIREBEE_YELLOW_LED, FIREBEE_LED_ON);
            } else {
                /* power state verified */
                if (power_status & (POWER_GOOD_1 << poe_port)) {
                    /* Power on. Turn on green LED */
                    firebee_led(iface, ext_start_port + poe_port, 
                                FIREBEE_GREEN_LED, FIREBEE_LED_ON);
                } else {
                    /* Power off. Turn off LED */
                    firebee_led(iface, ext_start_port + poe_port, 
                                FIREBEE_YELLOW_LED | FIREBEE_GREEN_LED, 
                                FIREBEE_LED_OFF);
                }
            } /* endof if (power_status) */
        } /* endof for */
    }
}



/* ******************************************************************
 *
 * Function: firebee_poe_loop_detect
 *
 * Description: loop detect for IEEE phone(debug only)
 *
 * Input:    iface - Pointer to interface data structure.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
int firebee_poe_loop_detect (hwic_iface_t *iface)
{
    if (firebee_poe_sku == FALSE) {     /* w/o PoE, skip the test */
        printf("This Firebee is not PoE SKU.\n");
        return (PASSED);
    }

    if (is_poe_present(iface) == FALSE) {
        printf("In Line Power card is not installed\n");
        return (FAILED);
    }

#if POE_DEBUG
    if ((check_poe_psu_present(POE_PSU_ONE, QUICK_MODE) == FALSE) && 
        (check_poe_psu_present(POE_PSU_TWO, QUICK_MODE) == FALSE)) {
#else
    if ((has_poe_psu(POE_PSU_ONE) == FALSE) && (has_poe_psu(POE_PSU_TWO) == FALSE)) {
#endif
        printf("\n WARNING, WARNING, Skipping POE loop detection.\n"
               " ILP power supply may not installed.\n");
        printf(" If ILP power supply is present then there is an ISSUE.\n"
               " Please check the power supply and ILP modules.\n");
        return (PASSED);
    }

    if (detect_ieee_phone(iface, POE_I2C_ADDR0) == FAILED) {
        return (FAILED);
    }

    if (iface->cookie_id == HWIC_8GE) {
        if (poe_si_flag_read() == TRUE) {
            if (detect_ieee_phone(iface, POE_I2C_ADDR2) == FAILED) {
                return (FAILED);
            }
        } else {
        if (detect_ieee_phone(iface, POE_I2C_ADDR1) == FAILED) {
            return (FAILED);
        }
    }
    }

    return (PASSED);

}

#endif






/*
 *------------------------------------------------------------------
 * $Log: dreamliner_poe.c,v $
 * Revision 1.4  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.3  2016/04/20 07:06:40  benchen2
 * merge tachi branch into main trunk
 *
 * Revision 1.2.4.3  2016/04/18 08:57:39  alpeng
 * fix for prrq
 *
 * Revision 1.2.4.2  2015/12/17 03:46:30  alpeng
 * support dreamliner nc and poe
 *
 * Revision 1.2.4.1  2015/08/17 02:33:03  alpeng
 * first check in for tachi-intel test; fix smart_cookie.c and free.h
 *
 * Revision 1.2  2015/02/27 10:02:20  iachang
 *
 * Add support dreamliner NIM
 *
 * Revision 1.1.6.2  2015/02/14 07:13:53  iachang
 * Dreamliner Diag sync with main trunk.
 *
 * Revision 1.1.4.4  2015/02/12 02:12:45  iachang
 * Added PoE init before into POE Utilities.
 * 
 * Revision 1.1.4.3  2015/02/06 10:34:31  iachang
 * Moved PoE init function from board init avoid user can't into module menu.
 * 
 * Revision 1.1.4.2  2015/01/28 22:59:21  iachang
 * Dreamliner-branch2 initial check-in.
 * 
 * Revision 1.1.2.3  2015/01/28 20:39:17  iachang
 * Fixed compile error
 * 
 * Revision 1.1.2.1  2014/12/02 08:04:11  iachang
 * Dreamliner Diag initial check-in.
 *------------------------------------------------------------------
 * $Endlog$
 */
