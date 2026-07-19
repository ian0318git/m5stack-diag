/* $Id: diag_barometer_lib.c,v 1.2 2016/04/20 11:25:26 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_barometer_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_barometer_lib.c - barometer Library Function
 *
 * July 2015, benchen2
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
#include "diag_barometer_lib.h"
#include "platform_i2c.h"
#include "diag_mcu_lib.h"
#include "assert.h"

uint32_t alt_sensor_read (n2g_i2c_if_t *, uint32_t);
uint32_t alt_sensor_write (n2g_i2c_if_t *, uint32_t);

int get_alt_sensor_i2c_struct (n2g_i2c_if_t *);


/*******************************************************************************
 *
 * Function   : alt_sensor_read
 * Description: To read the expected offset register data out.
 * Inputs     : *i2c_if - Pointer to the expected I2C interface structure
 *              offset - the offset of the expected register to read
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
 uint32_t alt_sensor_read (n2g_i2c_if_t *i2c_if, uint32_t offset)
{
    uint32_t rc = FAILED;

    if (!i2c_if->buf) {
        assert(!"env_read: buf is null");
    }

    i2c_if->offset = offset;

    rc = n2g_i2c_read(i2c_if);
    if (rc != RC_I2C_OP_OK) {
        /* Unable to read data */
        printf("*** %s: Unable to read %s Register 0x%02x(rc = %#x).\n",
               __FUNCTION__, i2c_if->dev_name, i2c_if->offset, rc);
        return (FAILED);
    }

    msleep(REN_I2C_PROC_TIME);  /* I2C cycle time */

    return (PASSED);
}
/*******************************************************************************
 *
 * Function   : alt_sensor_write
 * Description: To write the data into expected register.
 * Inputs     : *i2c_if - Pointer to the expected I2C interface structure
 *              offset - the offset of the expected register to write
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
uint32_t alt_sensor_write (n2g_i2c_if_t *i2c_if, uint32_t offset)
{
    uint32_t rc = FAILED;

    if (!i2c_if->buf) {
        assert(!"env_write: buf is null");
    }

    i2c_if->offset = offset;

    rc = n2g_i2c_write(i2c_if);
    if (rc != RC_I2C_OP_OK) {
        msleep(REN_I2C_PROC_TIME);  /* Env MCU I2C cycle time */
        return (FAILED);
    }

    msleep(REN_I2C_PROC_TIME);  /* Env MCU I2C cycle time */
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : get_alt_sensor_i2c_struct
 * Description: To get Altitude Sensor I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int get_alt_sensor_i2c_struct (n2g_i2c_if_t *alt_sensor_i2c)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)platform_fpga_get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
    		                                          MB_I2C_ADDR_BAROMETER);

    if (tmp == NULL) {
        printf("%s: Failed to get Altitude Sensor I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(alt_sensor_i2c, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}

/*---------------------------------------------------------------
$Log: diag_barometer_lib.c,v $
Revision 1.2  2016/04/20 11:25:26  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/07/31 07:08:57  hondwang
barometer library

$Endlog$
*/
