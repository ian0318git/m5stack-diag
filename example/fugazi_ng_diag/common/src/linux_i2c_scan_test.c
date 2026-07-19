/* $Id: linux_i2c_scan_test.c,v 1.2 2019/07/11 12:34:40 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_i2c_scan_test.c,v $
 *-----------------------------------------------------------------------------
 * linux_i2c_scan_test.c - For I2C test
 *
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "proto.h"
#include "queryflags.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "i2c_dev.h"
#include "linux_i2c_scan_test.h"

/*
 * Functional prototype
 */
int linux_i2c_scan_test(int, n2g_i2c_if_t *, int);

/*****************************************************************************
 *
 * Function   : linux_i2c_scan_test
 *
 * Description: scan all i2c devices according passing table
 *
 * Inputs     : option ,for future use.
 *              cpu_i2c_dev , passing table from platform
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int linux_i2c_scan_test (int option, n2g_i2c_if_t *cpu_i2c_dev, int num_i2c_dev)
{
    n2g_i2c_if_t i2c_if[num_i2c_dev];
    int ret_val = FAILED;
    uint32_t reg_val = 0;
    uint32_t ix, jx = 0;

    while (cpu_i2c_dev[jx].size != 0) {
        /*
         * Get I2C device structure
         */
        memcpy(&i2c_if[jx], &cpu_i2c_dev[jx], sizeof(n2g_i2c_if_t));
        i2c_if[jx].buf = (char *) &reg_val;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("I2C bus %2d, Mux %d, %-29s(0x%.2X)... ",
                   i2c_if[jx].i2c_bus_type, i2c_if[jx].mux, i2c_if[jx].dev_name,
                   (i2c_if[jx].i2c_dev << 1));
        } else {
            printf("[%2d] I2C_%d: %s\n ",
                   jx, i2c_if[jx].i2c_bus_type, i2c_if[jx].dev_name);
        }

        /*
         * Read I2C device Register 0
         */
        for (ix = 0; ix < MAX_RETRY; ix++) {
            ret_val = n2g_i2c_read(&i2c_if[jx]);
            if (ret_val != PASSED) {
                printf("I2C scan retry %d\n", ix);
            } else {
                break;
            }
            msleep(DELAY_I2C_SCAN_RETRY);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Done\n");
        }
        jx++;
    }

    if (ret_val != PASSED) {
        cterr('f', 0, "%s fail %s", i2c_if[jx].dev_name, i2c_err_str(ret_val));
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/*-------------------------------------------------
$Log: linux_i2c_scan_test.c,v $
Revision 1.2  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

$Endlog$
*/
