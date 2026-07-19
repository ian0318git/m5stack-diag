/* $Id: platform_sfp_cookie.c,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/platform_sfp_cookie.c,v $
 *------------------------------------------------------------------
 * platform_sfp_cookie.c
 *
 * Description: Wallander SFP Cookie I2C device.
 *              This file is ported from Overlord. 
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include "endians.h"
#include "common.h"
#include "platform_sfp_cookie.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "cross_platform.h"
#include "pca9545a.h"
#include "platform_i2c.h"
//#include "menu.h"
#include "n2g_api_rc.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "diag_fpga_lib.h"
#include "platform_eth.h"

/******************************************************************************
 *                                   Externs
 ******************************************************************************/
extern int get_num_ports(void);
extern int init_mux(int);
extern int set_mux_channel(n2g_i2c_dev_t *, uint8_t);

/****************************************************************************** 
 *                               Function prototypes
 ******************************************************************************/

int dump_sfp_eeprom(int);

/****************************************************************************** 
 *                                 Global variables
 ******************************************************************************/

static uchar mux9545_ports[] = {MUX9545_PORT0_MASK, MUX9545_PORT1_MASK};


/******************************************************************************
 *
 * function   : sfp_eeprom_dp_util
 * Description: Show sfp module content.
 * Inputs     : sfp - SFP I2C device number
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/

int sfp_eeprom_dp_util()
{
    int sfp = 0;

    sfp = gethex_answer("\nEnter port:",
               0, 0, 1);

    if (dump_sfp_eeprom(sfp)) {
        perror("\n SFP EEPROM dump failed.\n ");
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * function   : dump_sfp_eeprom
 * Description:	Show sfp module content.
 * Inputs     : sfp - SFP I2C device number
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int dump_sfp_eeprom (int sfp)
{
    uint32 rv;
    uchar burst_buf[I2C_BURST_SIZE];
    uint sfp_i2c_addr, reg_addr;
/*    int fpga_addr;
    uchar write_data;
    uchar read_data;*/
//     n2g_i2c_dev_t i2c_mux;
    n2g_i2c_dev_t i2c_sfp;
    int ix, max_port;

    /* Check if the SFP is valid */
    max_port = get_num_ports() / 2;
    if (sfp >= max_port) {
        cterr('f', 0, "SFP-%d not supported in SKU %d.", sfp, max_port);
        return (FAILED);
    }

    if (is_sfp_present(0, sfp) == FALSE) {
        printf("SFP-%d is not detected\n", sfp);
        return (PASSED);
    }
    if (show_sfp_status(0, sfp) == FAILED) {
        printf("Failed to read SFP-%d status\n", sfp);
        return (FAILED);
    }

    sfp_i2c_addr = CAVIUM_I2C_SFP;

    printf("open_i2c(&i2c_sfp, %#x, CPU_I2C0)\n", sfp_i2c_addr);

    /* Open the Cavium I2C bus 0 */
    if (open_i2c(&i2c_sfp, sfp_i2c_addr, CPU_I2C0) == FAILED) {
        cterr('f', 0, "Fail to open the i2c interface.");
        return (FAILED);
    }

    /* Setup Mux channel */
    if (set_mux_channel(&i2c_sfp, mux9545_ports[sfp]) != PASSED) {
        cterr('f', 0, "%s:%d Failed to set Mux channel %d.",
                      __FUNCTION__, __LINE__, sfp);
        return (FAILED);
    }

    /* Open the Cavium I2C bus 0 again */
    if (open_i2c(&i2c_sfp, sfp_i2c_addr, CPU_I2C0) == FAILED) {
        cterr('f', 0, "Fail to open the i2c interface.");
        return (FAILED);
    }

    /* Start to read SFP EEPROM */
    printf("SFP-%d EEPROM contents:\n\n", sfp);

    printf("0x00: ");
    for (reg_addr = 0; reg_addr < SFP_EEPROM_SIZE; reg_addr += I2C_BURST_SIZE) {
        if (reg_addr && ((reg_addr % 16) == 0)) {
            printf("\n");
            printf("0x%02x: ", reg_addr);
        }

        rv = read_i2c_reg(&i2c_sfp, (uchar *)&burst_buf, reg_addr,
                          I2C_BURST_SIZE);
        if (rv != PASSED) {
            printf("Read reg fail at offset 0x%.8x\n", reg_addr);
            return (FAILED);
        } else {
            for (ix = 0; ix < I2C_BURST_SIZE; ix++) {
                printf("%02x ", burst_buf[ix]);
            }
            printf("\t\t");
            for (ix = 0; ix < I2C_BURST_SIZE; ix++) {
                uchar ch = burst_buf[ix];
                if ((ch < 0x20) || (ch > 0x7e)) {
                    printf(".");
                } else {
                    printf("%c", ch);
                }
            }
        }
    }
    printf("\n");

    return (PASSED);
}


/*------------------------------------------------------------------
 * $Log: platform_sfp_cookie.c,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *------------------------------------------------------------------
 */
