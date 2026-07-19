/* $Id: plug_host_fpga_lib.c,v 1.2 2019/01/10 06:36:25 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/plug_host_fpga_lib.c,v $
 *------------------------------------------------------------------
 * 
 * plug_host_fpga_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include "proto.h"
#include "types.h"
#include "nvmonvars.h"
#include "common_utils.h"
#include "cross_platform.h"
#include "endians.h"
#include "types.h"
#include "dev_object.h"     
#include "byteswap.h"
#include "linux_api.h"
#include "cookie_4.h"
#include "plug_common_host_impl.h"
#include "plug_common_lib.h"
#include "plug_testcard_host_impl.h"
#include "platform_stub.h"
#include "plug_testcard_phy.h"
#include "plug_host_fpga_lib.h"
#include "diag_sirius_fpga_lib.h"

int plug_host_slot_module_info(int, uint16_t);
static int plug_pwr_ctrl(int, int);
int diag_fpga_reg_bitops(uint, uint, uint);
int fpga_read_32_reg(uint, uint *);
int fpga_write_32_reg(uint, uint );


unsigned long base_plug_fpga;

extern int err_no;
extern int i2c_status;


static plug_module_sku_info plug_module_sku_tbl[] = {
    {"PLUGGABLE_TEST_CARD",   PLUGGABLE_TEST_CARD},
    {"PLUGGABLE_LTE_EM",      PLUGGABLE_LTE_EM},
    {"PLUGGABLE_LTE_WP7601",  PLUGGABLE_LTE_WP7601},
    {"PLUGGABLE_LTE_WP7603",  PLUGGABLE_LTE_WP7603},
    {"PLUGGABLE_LTE_WP7607",  PLUGGABLE_LTE_WP7607},
    {"PLUGGABLE_LTE_WP7608",  PLUGGABLE_LTE_WP7608},
    {"PLUGGABLE_LTE_WP7609",  PLUGGABLE_LTE_WP7609},
    {"PLUGGABLE_SERIAL",      PLUGGABLE_SERIAL},
    { NULL, 0x0000}
};


/*******************************************************************************
 *
 * Function   : plug_host_slot_module_info 
 * Description: show pluggable slot module info by cookie 
 * Inputs     : slot number
 *              module cookie
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_host_slot_module_info (int slot, uint16_t cookid)
{
    plug_module_sku_info *plugp;
    plugp = plug_module_sku_tbl;
    /* Search match platform cookie table */
    while (plugp->plug_module_name != NULL) {
        if (plugp->cook_contype == cookid) {
            printf("Slot %1d: %s cookie id = 0x%4x.\n", slot, plugp->plug_module_name, cookid);
            break;
        }
        plugp++;
    }
    if (plugp->plug_module_name == NULL) {
        printf("*** WARNING: Could not find correct PLUG module SKU info.\n");
        return (FAILED);
    }
    return (PASSED);

}

/*******************************************************************************
 * Function   : diag_plug_pwr_on_util
 * Description: Function to power on pluggable module
 * Inputs     : None 
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
int diag_plug_pwr_on_util (void)
{   
    int slot;
    int pwr_opt;

    pwr_opt = PLUG_SLOT_PWR_ON;
    slot = getdec_answer("Power on which slot? (1-2):", PLUG_SLOT_1,
                         PLUG_SLOT_1, PLUG_SLOT_2);
    plug_pwr_ctrl(slot, pwr_opt);

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_plug_pwr_off_util
 * Description: Function to power off pluggable module
 * Inputs     : None 
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
int diag_plug_pwr_off_util (void)
{   
    int slot;
    int pwr_opt;

    pwr_opt = PLUG_SLOT_PWR_OFF;
    slot = getdec_answer("Power off which slot? (1-2):", PLUG_SLOT_1,
                         PLUG_SLOT_1, PLUG_SLOT_2);
    
    plug_pwr_ctrl(slot, pwr_opt);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_pwr_ctrl
 * Description: Function to power on/off pluggable module
 * Inputs     : slot - which pluggable slot
 *              value - 1 for power on 
 *                      0 for power off
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
static int plug_pwr_ctrl (int slot, int value)
{   
    uint reg;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);
    if (value == PLUG_SLOT_PWR_OFF) {
        diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_PWR_EN_BIT);
    } else {
        diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_PWR_EN_BIT);
    }

    return (PASSED);
}



/**************************************************************************
 *
 * Name: plug_i2c_act2_reset
 *
 * Description: This function implementes a reset to Quack chip by
 *              reset the line for 50ms then unreset it
 *
 * Inputs: con - pointer to sc_context
 *
 * Outputs: None
 *
 *************************************************************************/
void plug_i2c_act2_reset (sc_context *con_p)
{
    unsigned int reg, reset, slot; 
    if (con_p->type == PLUGGABLE_CARD)  {
        slot = con_p->slot;
        switch (slot) {
        case PLUG_SLOT_1: 
            reg = FPGA_PLUG1_STSCTL_REG; 
            reset = PLUG_I2C_RESET; 
        break; 
        case PLUG_SLOT_2: 
            reg = FPGA_PLUG2_STSCTL_REG; 
            reset = PLUG_I2C_RESET; 
        break; 
        }    
        printf("Resetting ACT2 PLUG%d...", slot);
        fflush(stdout);
 
        plug_fpga_reg_or(reg, reset); 
        msleep(ACT2_RESET_UNRESET_DELAY);
        plug_fpga_reg_nand(reg, reset); 
 
        /* ACT2 unreset delay implement in tam_lib_platform_read */
        msleep(ACT2_UNRESET_DELAY);
        printf("Done\n");
        fflush(stdout);
    }
    return;
}

/*-------------------------------------------------
 * $Log: plug_host_fpga_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
