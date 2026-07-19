/* $Id: platform_vtg_mntr.c,v 1.2 2019/08/06 06:56:14 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_vtg_mntr.c,v $
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
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
    	printf("margined -3%%\n");
    } else if (vmc & VTG_MNTR_3_3V_MRGN_HI) {
    	printf("margined +3%%\n");
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

/*------------------------------------------------------------------
$Log: platform_vtg_mntr.c,v $
Revision 1.2  2019/08/06 06:56:14  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.2  2018/12/27 07:49:53  alpeng
based on prrq comments to refine these files for message display

Revision 1.1.2.1  2018/06/22 08:05:20  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:38  alpeng
porting neptune x86 to curie

Revision 1.2  2018/05/18 09:25:01  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.1  2017/02/18 03:37:51  meho
Added voltage margin utility.

Revision 1.3  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.2  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.8  2012/11/28 18:19:10  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.7  2012/11/21 19:47:22  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.6  2012/09/26 18:02:15  palin2
Uniformed the print out format of I2C devices defult tests.

Revision 1.5  2012/08/13 08:02:46  alpeng
clean up useless message

Revision 1.4  2012/06/28 06:11:38  palin2
Change register dump display format.

Revision 1.3  2012/05/30 16:45:03  palin2
Clean up compile warnings.

Revision 1.2  2012/03/28 00:38:25  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
