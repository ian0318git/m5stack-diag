/* $Id: diag_temp_sensor_lib.c,v 1.2 2016/04/20 11:25:24 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_temp_sensor_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_temp_sensor_lib.c - Temp sensor Library Function
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
#include "diag_temp_sensor_lib.h"
#include "platform_i2c.h"


int diag_temp_sensor_reg_write(uint32_t, uint32_t, uint16_t);
int diag_temp_sensor_reg_read(uint32_t, uint32_t, uint16_t *);
int get_temp_sensor_device_addr(int);
static int diag_get_temp_sensor_i2c_struct (n2g_i2c_if_t *, uint32_t);

int diag_temp_sensor_reg_read (uint32_t i2c_offset, uint32_t offset, uint16_t *data_in)
{
    int rc;
    n2g_i2c_if_t i2c_if;

    rc = diag_get_temp_sensor_i2c_struct(&i2c_if, i2c_offset);

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

    *data_in = DSWAP2(*data_in);

    msleep(REN_I2C_PROC_TIME);

    return (PASSED);
}

int diag_temp_sensor_reg_write (uint32_t i2c_offset, uint32_t offset, uint16_t data_out)
{
    int rc;
    n2g_i2c_if_t i2c_if;

    rc = diag_get_temp_sensor_i2c_struct(&i2c_if, i2c_offset);

    if (rc != PASSED) {
        return (FAILED);
    }

    data_out = DSWAP2(data_out);

    i2c_if.buf = (char *)&data_out;
    i2c_if.size = sizeof(uint16_t);
    i2c_if.offset = offset;

    rc = n2g_i2c_write(&i2c_if);

    if (rc != RC_I2C_OP_OK) {
        msleep(REN_I2C_PROC_TIME);
        return (FAILED);
    }

    /* I2C cycle time */
    msleep(REN_I2C_PROC_TIME);

    return (PASSED);
}


static int diag_get_temp_sensor_i2c_struct (n2g_i2c_if_t *temp_sensor_i2c, uint32_t i2c_offset)
{
    n2g_i2c_if_t *tmp;

    tmp = (n2g_i2c_if_t *)platform_fpga_get_n2g_i2c_if(I2C_CTRL_TWO,
                                                       I2C_MUX_ZERO,
                                                       GET_ADDRESS(i2c_offset));

    if (tmp == NULL) {
        printf("%s: Failed to get Temp sensor I2C interface structure\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(temp_sensor_i2c, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}

int get_temp_sensor_device_addr(int option)
{
	int tpm75_offset;

    switch (option) {
	case 0:
	    tpm75_offset = GET_TPM75_DEV_ADDR(MB_I2C_ADDR_TEMP_INLET_U27);
        break;
	case 1:
	    tpm75_offset = GET_TPM75_DEV_ADDR(MB_I2C_ADDR_TEMP_INLET_U29);
	    break;
	case 2:
        tpm75_offset = GET_TPM75_DEV_ADDR(MB_I2C_ADDR_TEMP_OUTLET_U39);
	    break;
	case 3:
	    tpm75_offset = GET_TPM75_DEV_ADDR(MB_I2C_ADDR_TEMP_OUTLET_U337);
		break;
	default:
	    printf("Please type the right choice!\n");
	}

    return(tpm75_offset);
}

/*---------------------------------------------------------------
$Log: diag_temp_sensor_lib.c,v $
Revision 1.2  2016/04/20 11:25:24  benchen2
add tachi fru portion

Revision 1.1.2.4  2015/09/17 13:05:10  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.3  2015/08/22 06:09:39  benchen2
Add temp sensor test item

Revision 1.1.2.2  2015/08/21 11:31:21  benchen2
add temperature sensor utility

Revision 1.1.2.1  2015/07/31 07:25:15  hondwang
temp sensor lib

$Endlog$
*/


