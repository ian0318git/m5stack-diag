/* $Id: platform_vtg_mntr.c,v 1.1 2020/01/09 01:02:05 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_vtg_mntr.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_vtg_mntr.c
 * 
 * Description: Operation-Overlord Voltage Monitor I2C device.
 *
 *		According to EDCS-618748 (Xformers Power Sequencer HFS),
 *		"Diagnostics must consider a board to have failed if either
 *		the VOLTAGE_FAULT_DURING_POWER_UP or the
 *		VOLTAGE_FAULT_DURING_OPERATION bits are set in the Status
 *		Register. In additional to flagging the particular platform
 *		as "FAILED", the diagnostic code should dump all the Power
 *		Sequencer registers to aid in debug and isolation of the
 *		problem."
 *
 *		The Power Sequencer has a PWR_MCU_RST_L (power sequencer
 *		reset signal) pin, and a GFYM_HRESET_OUT_L ("This is the
 *		software reset signal") pin. Only GFYM_HRESET_OUT_L from
 *		Goofy can reset the Power Sequencer.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 * 
 *  Notes: this voltage monitor is porting from 
 *         Informer platform_pwr_seq.c 
 *         
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "endians.h"
#include "common.h"
#include "nvmonvars.h"
#include "platform_vtg_mntr.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "menu.h"
#include "n2g_api_rc.h"
#include "byteswap.h"
#include "i2c_address.h"
#include "dash_fpga.h"
#include "goofy_i2c.h"
#include "platform_margin_utils.h"

#define WD_BYPASS /* */
extern int do_all_menu_items(struct menuinfo *);
extern uint32 cterr_db_print(char *fmtptr, ...);

static void show_volt_margin(ren_t);
static void vtg_mntr_read_reg(uint32_t *);
static void vtg_mntr_write_reg(uint32_t);

/*********************************************************************
 *
 * Function:	vtg_mrgn
 *
 * Description:	Margin read/write.
 *
 * Inputs:	option - Refer to voltage_margin_t enum.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int vtg_mrgn(int option)
{
    uint32_t rc = PASSED;
    uint32_t old_data, new_data; 
    vtg_mntr_read_reg(&old_data);
  
#ifdef VM_DEBUG
  printf("read from reg:vtg_margin_ctrl:  old_data = %x \n", old_data);
#endif

    switch(option) {
        case VTG_MRGN_GET_3_3V:
	        new_data = old_data;
	        show_volt_margin(old_data);
        break;
        case VTG_MRGN_SET_3_3V_NORM:
	        new_data = old_data & (~VTG_MNTR_3_3V_MRGN_LO);
	        new_data &= ~(VTG_MNTR_3_3V_MRGN_HI);
        break;
        case VTG_MRGN_SET_3_3V_HI:
            new_data = old_data & (~VTG_MNTR_3_3V_MRGN_LO);
	        new_data |= VTG_MNTR_3_3V_MRGN_HI;
        break;
        case VTG_MRGN_SET_3_3V_LO:
	        new_data = old_data & (~VTG_MNTR_3_3V_MRGN_HI);
	        new_data |= VTG_MNTR_3_3V_MRGN_LO;
        break;
        default:
	        new_data = old_data;
	        cterr('f', 0, "vtg_mrgn() Invalid option %#x", option);
	        rc = FAILED;
	    break;
    } /* endof switch */


    if (new_data != old_data) {
        vtg_mntr_write_reg(new_data);
    }
    
    return (rc);
}

/*********************************************************************
 *
 * Function:	show_volt_margin
 *
 * Description:	Display voltage margin.
 *
 * Inputs:	vmc - Voltage Margin Control register read.
 *
 * Outputs:	None
 *
 *********************************************************************
 */
static void show_volt_margin(ren_t vmc)
{
    printf("3.3V: ");
    if (vmc & VTG_MNTR_3_3V_MRGN_LO) {
        printf("margined -5%%\n");
    } else if (vmc & VTG_MNTR_3_3V_MRGN_HI) {
        printf("margined +5%%\n");
    } else {
        printf("not margined\n");
    }
}

/*********************************************************************
 *
 * Function:	vtg_mntr_read_reg
 *
 * Description:	For show register using.
 *
 * Inputs:	d32 - pass the data with 32 bits.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static void vtg_mntr_read_reg(uint32_t *data)
{
    assert(dash_fpga);

    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    *data = sys->vtg_mrg_ctrl;
}

/*********************************************************************
 *
 * Function:	vtg_mntr_write_reg
 *
 * Description:	For alter register using.
 *
 * Inputs:	d32 - pass the data with 32 bits.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static void vtg_mntr_write_reg(uint32_t data)
{
    assert(dash_fpga);
    /* write magic value(0xc5c0) for the margin bits to take effect. */
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    sys->vtg_mrg_ctrl = ((0xc5c0 << 16) | data);
}


int vtg_get_version (uint16_t *version)
{
    vtg_mntr_read_reg((uint32_t *)version);
    return PASSED;
}

/*
 *-----------------------------------------------------------------------------
$Log: platform_vtg_mntr.c,v $
Revision 1.1  2020/01/09 01:02:05  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
