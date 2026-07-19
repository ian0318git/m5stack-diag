/* $Id: diag_mcu_lib.c,v 1.2 2016/04/20 11:25:27 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_mcu_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_mcu_lib.c - MCU Library Function
 *
 * July 2015, Times Huang  
 * 
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "proto.h"
#include "byteswap.h"
#include "i2c_api.h"
#include "diag_i2c_api.h"
#include "diag_mcu_lib.h"
#include "platform_i2c.h"

int diag_mcu_reg_write(uint32_t, uint16_t);
int diag_mcu_reg_read(uint32_t, uint16_t *);

static int diag_get_mcu_i2c_struct(n2g_i2c_if_t *); 

int diag_mcu_reg_read (uint32_t offset, uint16_t *data_in)
{
    int rc;
    n2g_i2c_if_t i2c_if;

    rc = diag_get_mcu_i2c_struct(&i2c_if);

    if (rc != PASSED) {
        return (FAILED);
    }

    i2c_if.buf = (char *)data_in;
    i2c_if.size = sizeof(uint16_t);
    i2c_if.offset = offset;

    rc = n2g_i2c_read(&i2c_if);
    
    if (rc != RC_I2C_OP_OK) {
        printf("%s: Unable to read. rc=0x%08x\n", __FUNCTION__, rc);
        return (FAILED);
    }

    msleep(REN_I2C_PROC_TIME);

    return (PASSED);
}

int diag_mcu_reg_write (uint32_t offset, uint16_t data_out)
{
    int rc;
    n2g_i2c_if_t i2c_if;
    uint16_t tmp;

    rc = diag_get_mcu_i2c_struct(&i2c_if);

    if (rc != PASSED) {
        return (FAILED);
    }

    tmp = data_out;

    i2c_if.buf = (char *)&tmp;
    i2c_if.size = sizeof(uint16_t);
    i2c_if.offset = offset;

    rc = n2g_i2c_write(&i2c_if);

    if (rc != RC_I2C_OP_OK) {
        msleep(REN_I2C_PROC_TIME);
        return (FAILED);
    }

    /* Env MCU I2C cycle time */
    msleep(REN_I2C_PROC_TIME);

    return (PASSED);
}


static int diag_get_mcu_i2c_struct (n2g_i2c_if_t *mcu_i2c)
{
    n2g_i2c_if_t *tmp;

    tmp = (n2g_i2c_if_t *)platform_fpga_get_n2g_i2c_if(I2C_CTRL_TWO, 
                                                       I2C_MUX_ZERO,
                                                       MB_I2C_ADDR_ENV_MCU);

    if (tmp == NULL) {
        printf("%s: Failed to get Env MCU I2C interface structure\n", 
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(mcu_i2c, tmp, sizeof(n2g_i2c_if_t));
    
    return (PASSED);
}

/*---------------------------------------------------------------
$Log: diag_mcu_lib.c,v $
Revision 1.2  2016/04/20 11:25:27  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/09/25 02:18:24  tirawan
Correct MCU reg read/write (not to byte swap) and display MCU version

Revision 1.1.2.1  2015/07/26 06:02:21  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming


$Endlog$
*/
