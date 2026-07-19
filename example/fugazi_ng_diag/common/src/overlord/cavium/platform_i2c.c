/* $Id: platform_i2c.c,v 1.4 2013/11/26 08:40:37 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/platform_i2c.c,v $
 *-----------------------------------------------------------------------------
 * platform_i2c.c - Informers I2C utility menu.
 *
 * Sept. 2007, Simon Yen
 *
 * Copyright (c) 2007-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "proto.h"
#include "platform_i2c.h"
#include "platform_sfp_cookie.h"
#include "i2c_api.h"
#include "i2c_dev.h"
#include "nvmonvars.h"


/*
 *  Externs
 */
extern int32_t cavium_i2c_fd0;
extern int32_t cavium_i2c_fd1;

/* for utilities submenu. */

/*
 * Functional prototype
 */
int check_i2c_fd(n2g_i2c_dev_t *);
int cavium_i2c_scan_test(void);

/*
 *  Globals  
 */
static n2g_i2c_dev_t cavium_i2c_dev[] = {
    {
        /* device name DIMM */
        .bus_no = CPU_I2C0, 
        .dev_addr = MB_I2C_ADDR_DIMM0,
        .rd_hd_size = 1,
        .wr_hd_size = 1,
        .fp = 0,
    },
    {
        /* device name 124mux */
        .bus_no = CPU_I2C0, 
        .dev_addr = OVLD_CAVIUM_MUX_I2C_ADDR,
        .rd_hd_size = 0,
        .wr_hd_size = 0,
        .fp = 0,
    },
};


/* to check i2c file descriptor is open or not */
int check_i2c_fd (n2g_i2c_dev_t *dev){

    uint32_t rc = FAILED;

    if (dev->dev_addr == MB_I2C_ADDR_DIMM0) {
        /* Set I2C device to SLAVE mode */
        if (cavium_i2c_fd0 <= 0) {
            cterr('f', 0, "/dev/i2c-octeon.1/ is not opened correctly.");
            return (FAILED);
        } else {
            /* Set I2C device to SLAVE */
            if ((rc = ioctl(cavium_i2c_fd0, I2C_SLAVE, dev->dev_addr)) < 0) {
                cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          dev->dev_addr, rc);
                return (FAILED);
            } else {
                dev->fp = cavium_i2c_fd0;
            }
        }
    } else if (dev->dev_addr == OVLD_CAVIUM_MUX_I2C_ADDR) {
        if (cavium_i2c_fd1 <= 0) {
            cterr('f', 0, "/dev/i2c-octeon.1/ is not opened correctly.");
            return (FAILED);
        } else {
            /* Set I2C device to SLAVE */
            if ((rc = ioctl(cavium_i2c_fd1, I2C_SLAVE, dev->dev_addr)) < 0) {
                cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          dev->dev_addr, rc);
                return (FAILED);
            } else {
                dev->fp = cavium_i2c_fd1;
            }
        } 
    } else {
      /* for future using */ 
    }

   return (PASSED);

}

/**********************************************************************
 *
 * Function: cavium_i2c_scan_test
 *
 * Description: perform i2c read on the devices which are connect 
 *              with I2C on cavium.
 *
 * Input: None
 *
 * Return: pass/fail
 */

int cavium_i2c_scan_test (void)
{
    n2g_i2c_dev_t  i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      reg_val = 0, ret_val = FAILED, fail_ctr = 0;
    uint8_t       now_test = 0, test_end = 0;
    uchar tname[32] = "Cavium I2C scan";

    testname("%s", tname);

    /* Setup end of test by calculate all FPGA I2C device number */
    test_end = (sizeof(cavium_i2c_dev)/sizeof(n2g_i2c_dev_t));

    for (now_test = 0; now_test < test_end; now_test++) {

        prpass(testpass, "Device %d of %d, ", (now_test + 1), test_end);  
   
        /* Get I2C device structure */
        memcpy(&i2c_dev, &cavium_i2c_dev[now_test], sizeof(n2g_i2c_dev_t));

        /* check fd of device */ 
        ret_val = check_i2c_fd(&i2c_dev);
        if (ret_val != PASSED)
            return FAILED;  /* the cterr is set on check_i2c_read. */

        /* Setup the interface struct for I2C API read */
        i2c_if.offset = 0;
        i2c_if.i2c_ctrl = 0;/* not sure need to specified */ 
        i2c_if.mux = 0;  /* not sure need to specified */
        i2c_if.size = sizeof(uint8_t);  
        i2c_if.buf = (char *)&reg_val;

        /* Read I2C device Register 0 */
        ret_val = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
        if (ret_val != PASSED) {
            fail_ctr++;
            cterr('f',0,"%s addr 0x%x failed\n", tname, i2c_dev.dev_addr);

        } else {
            /* if mux pass, then test SFP which is at the end of i2c */ 
            if(i2c_dev.dev_addr == OVLD_CAVIUM_MUX_I2C_ADDR){
                ret_val = sfp_i2c_test_warp(); 
            } else { 
                printf("passed");
            }
        }
    }
   
    if(fail_ctr == 0)
        printf("\n"); 

    return (ret_val);
}



/* end of file */

/******** History ******** 
*---------------------------------------------------
$Log: platform_i2c.c,v $
Revision 1.4  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.3  2012/06/20 07:26:07  alpeng
including i2c scan test for SFP

Revision 1.2  2012/06/05 06:21:03  alpeng
clean up compiler warnings.

Revision 1.1  2012/05/30 09:36:54  alpeng
suppoted i2c scan test on cavium side, removing useless definition on platform_i2c.h


$Endlog$
*/
