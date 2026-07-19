/* $Id: platform_vtg_mntr.c,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_vtg_mntr.c,v $
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
 * Copyright (c) 2019-2021 by Cisco Systems, Inc.
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
#include <linux/types.h>
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
#include "platform_i2c.h"
#include "platform_pwr_seq.h"

#define WD_BYPASS /* */
extern int do_all_menu_items(struct menuinfo *);
extern uint32 cterr_db_print(char *fmtptr, ...);

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
    n2g_i2c_dev_t i2c_dev;
    uint32_t rc = PASSED;
    int CMD = CMD_OPERATION;

    /* Init device structure */
    if (init_tps536xx_i2c_struct(&i2c_dev, MB_I2C_TPS53659_3P3V) != PASSED) {
        printf("Init TPS536XX i2c_dev struct failed.");
        return (FAILED);
    }

    switch(option) {
        case VTG_MRGN_SET_3_3V_NORM:
             printf("\n--- Set Margin None ---\n");
             /* Write OPERATION(01h) - margin none is 0x80 */
             rc = i2c_smbus_write_byte_data(i2c_dev.fp, CMD, OP_MARGIN_NONE);
        break;
        case VTG_MRGN_SET_3_3V_HI:
             printf("\n--- Set Margin HI ---\n");
             /* Write OPERATION(01h) - margin high is 0xA4 */
             rc = i2c_smbus_write_byte_data(i2c_dev.fp, CMD, OP_MARGIN_HIGH);

        break;
        case VTG_MRGN_SET_3_3V_LO:
             printf("\n--- Set Margin LOW ---\n");
             /* Write OPERATION(01h) - margin low is 0x94 */
             rc = i2c_smbus_write_byte_data(i2c_dev.fp, CMD, OP_MARGIN_LOW);
        break;
        default:
	        cterr('f', 0, "vtg_mrgn() Invalid option %#x", option);
	        rc = FAILED;
	    break;
    } /* endof switch */
    
    return (rc);
}

/*********************************************************************
 *
 *
 * Function: show_mrgn
 *
 * Description:	Show Margin status.
 *
 * Inputs:	option - Refer to voltage_margin_t enum.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int show_mrgn(void)
{
    n2g_i2c_dev_t i2c_dev;
    uint32_t rc = PASSED;
    int CMD = CMD_OPERATION;
    uint8_t rdvale = 0;
    uint16_t pwseq_data = 0;
    uint32_t pwseq_offset;

    /* Init device structure */
    if (init_tps536xx_i2c_struct(&i2c_dev, MB_I2C_TPS53659_3P3V) != PASSED) {
        printf("Init TPS536XX i2c_dev struct failed.");
        return (FAILED);
    }

    rc = i2c_smbus_read_byte_data(i2c_dev.fp, CMD, &rdvale);

    if (rc != PASSED) {
        printf("Failed to read margin status");
        return (rc);
    }

    if (rdvale == OP_MARGIN_HIGH)
    {
        printf("\n--- Margin HI ---");
    } else if (rdvale == OP_MARGIN_LOW) {
        printf("\n--- Margin LOW ---");
    } else {
        printf("\n--- Margin None ---");
    }
    
    /* CSCvo59196-30: display additional current voltage.
     * Get current voltage level from Power sequenc register 
     * offset 0x30 "VP3P3 Voltage : Latest Reading" 
     */
    pwseq_offset = PWR_SEQ_3_3_LR;
    if ((rc = pwr_seq_read_reg(pwseq_offset, &pwseq_data)) != RC_I2C_OP_OK) {
        printf("\n%s: Unable to read Power Seq Regiser %#x.", __FUNCTION__, pwseq_offset);
        return (FAILED);
    }

    printf("(VP3P3 Voltage - Latest Reading %dmv [0x%02X:0x%04x])\n",
            pwseq_data, pwseq_offset, pwseq_data);
    
    return (rc);
}
/*-------------------------------------------------
 * $Log: platform_vtg_mntr.c,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.3  2021/04/29 01:33:41  pdoong
 * Add display additional current voltage info while display 'Show Current Voltage'
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.5  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.4  2019/05/13 07:35:28  letsai
 * Add utility to show current margin.
 *
 * Revision 1.1.6.3  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:37  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
