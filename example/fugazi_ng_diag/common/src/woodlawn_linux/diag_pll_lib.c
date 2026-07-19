/* $Id: diag_pll_lib.c,v 1.2 2013/10/08 08:48:29 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_pll_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_pll_lib.c - Woodlawn PLL Library
 *
 * February 2012, Times Huang
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include "diag_fpga_lib.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "dev_tmp421.h"
#include "i2c_dev.h"

int pll_reg_read(int, char *);
int pll_reg_write(int, char);


/******************************************************************************
 *
 * Function    : pll_reg_read
 * Description : PLL Register Read through Cavium I2C iface
 * Input       : addr  - register offset.
 *               buf   - read buffer
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int pll_reg_read (int addr, char *buf)
{
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_PLL;
    
    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C0) == FAILED) {
        return (FAILED);
    }

    /* Call the I2C common I2C api to read back the register value */
    return (read_i2c_reg(&i2c_dev, (uchar *)buf, addr, sizeof(fpga_p)));
}

/******************************************************************************
 *
 * Function    : pll_i2c_write
 * Description : PLL Register Write through Cavium I2C iface
 * Input       : addr  - register offset.
 *               data  - data for write
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int pll_reg_write (int addr, char data)
{
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_PLL;
    
    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C0) == FAILED) {
        return (FAILED);
    }

    /* Call the I2C common I2C api to write the register value */
    return (write_i2c_reg(&i2c_dev, (uchar *)&data, addr, sizeof(fpga_p)));
}

/*-------------------------------------------------
 * $Log: diag_pll_lib.c,v $
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:53  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:18  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/03/27 04:49:36  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.2  2012/10/24 10:36:55  leslie
 * Fix argument type.
 *
 * Revision 1.1  2012/09/05 22:16:48  leslie
 * Add Woodlawn PLL lib file.
 *
 * $Endlog$
 *-------------------------------------------------
 */
