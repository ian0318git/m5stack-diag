/* $Id: pca.c,v 1.6 2020/05/22 02:28:23 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/pca.c,v $
 *------------------------------------------------------------------
 * pca.c
 *
 * This file contains read/write route for pca chip
 * Copyright (c) 2009-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: mcharon
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include "proto.h"
#include "common.h"
#include "goofy_i2c.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "i2c_api.h"
#include <assert.h>

static n2g_i2c_if_t pca =
    {
        .dev_name = "PCA9557",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .size    = sizeof(uint8_t),
        .mux = I2C_MUX_ZERO, 
        .buf        = NULL,
    };


void
pca_init_i2c (void *dev)
{
    assert(dev);
    memcpy(dev, &pca, sizeof(n2g_i2c_if_t));
}

/**************************************************************************
 *
 * Name: io_port_8bit_i2c_read
 *
 * Description: This function reads I2C data from the 8-bit I/O port
 *              device PCA9554.
 *
 * Inputs: slot_num.
 *
 * Output: none.
 *
 *************************************************************************/
int
io_port_8bit_i2c_read (void *i2c, int32_t offset, uchar *data,  uchar flag)
{
    int rc;
    n2g_i2c_if_t *i2c_if = (n2g_i2c_if_t *)i2c;

    assert(i2c);
    /*
    printf("pca.c: i2c_ctrl %d; i2c_dev %#x; i2c_mux = %d; i2c_size = %d\n",
           i2c_if->i2c_ctrl, i2c_if->i2c_dev, i2c_if->mux, i2c_if->size);
    */
    i2c_if->offset = offset;

    if (!i2c_if->buf)
        i2c_if->buf = (char *)data;

    /* Goofy */
    rc = n2g_i2c_read(i2c_if);
    
    if (rc != RC_I2C_OP_OK) {
	if (flag == FALSE) {
	    cterr('f', 0, "io_port_8bit_i2c_read:i2c read failed: rc = 0x%x", rc);
	}
	return(FAILED);

    }
    memcpy(data, i2c_if->buf, sizeof(uchar));

    return (PASSED);
}

/**************************************************************************
 *
 * Name: io_port_8bit_i2c_write
 *
 * Description: This function write I2C data to the 8-bit I/O port
 *              device PCA9554.
 *
 * Inputs: slot_num.
 *
 * Output: none.
 *
 *************************************************************************/
int
io_port_8bit_i2c_write (void *i2c, uint32_t offset, uchar *data)
{
    int rc;
    n2g_i2c_if_t *i2c_if = (n2g_i2c_if_t *)i2c;
    /* Goofy */
    assert(i2c);
    /*
    printf("pca.c: i2c_ctrl %d; i2c_dev %#x; i2c_mux = %d; i2c_size = %d\n",
           i2c_if->i2c_ctrl, i2c_if->i2c_dev, i2c_if->mux, i2c_if->size);
    */
    i2c_if->offset = offset;

    if (!i2c_if->buf)
        i2c_if->buf = (char *)data;
    else
        memcpy(i2c_if->buf, data, sizeof(uchar));

    /* Goofy */
    rc = n2g_i2c_write(i2c_if);
    if (rc != RC_I2C_OP_OK) {
        cterr('f', 0, "io_port_8bit_i2c_write:i2c read failed: rc = 0x%x", rc);
	return(FAILED);

    }
    return (PASSED);

}

/******** History ******** 
$Log: pca.c,v $
Revision 1.6  2020/05/22 02:28:23  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.5  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.4  2012/09/18 19:19:54  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.3  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:22  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
