/* $Id: platform_i2c.c,v 1.3 2013/11/26 08:40:39 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_i2c.c,v $
 *-----------------------------------------------------------------------------
 * platform_i2c.c - Woodlawn I2C library
 *
 * Mar. 2012,
 * Copyright (c) 2007-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "endians.h"
#include <setjmp.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "platform_sfp_cookie.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "proto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nvmonvars.h"

/*
 *  Externs
 */
extern int32_t cavium_i2c_fd0;
extern int32_t cavium_i2c_fd1;

/******************************************************************************
 *  Global Variable
 *****************************************************************************/
char i2c_err_buf[I2C_ERR_BUF_SIZE];

/******************************************************************************
 * Functional prototype
 *****************************************************************************/
int read_i2c_reg(n2g_i2c_dev_t *, uchar *, uint, uchar);
int write_i2c_reg(n2g_i2c_dev_t *, uchar *, uint, uchar);
uint32_t open_i2c(n2g_i2c_dev_t *, uint, uint8_t);

int cavium_i2c_scan_test(void);
int pca9557_power_margin_ctrl(void);
int pca9557_power_margin_status(void);
/*
 * Functional prototype
 */
int check_i2c_fd(n2g_i2c_dev_t *);

/*
 *  Globals  
 */

/******************************************************************************
 *
 * Function    : open_i2c
 * Description : Open the Cavium I2C bus 1 interface
 * Input       : i2c_dev  - pointer to the I2C device
 *                  i2c_slave_addr - I2C slave physical address
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
uint32_t open_i2c (n2g_i2c_dev_t *i2c_dev, uint i2c_slave_addr, uint8_t i2c_bus)
{
    uint32_t rc = PASSED;
    int32_t cavium_i2c_fd;
    /* Setup I2C device data structure */
    i2c_dev->bus_no = i2c_bus;
    i2c_dev->dev_addr = i2c_slave_addr;
    i2c_dev->rd_hd_size = 1;
    i2c_dev->wr_hd_size = 1;
    
    /* Set I2C device to SLAVE */
    if ((cavium_i2c_fd = (i2c_bus == CPU_I2C0)? cavium_i2c_fd0 : cavium_i2c_fd1)) {
        if (cavium_i2c_fd <= 0) {
            cterr('f', 0, "/dev/i2c-octeon.%d is not opened correctly.", i2c_bus);
            return (FAILED);
        } else {
            if ((rc = ioctl(cavium_i2c_fd, I2C_SLAVE, i2c_dev->dev_addr)) < 0) {
                cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev->dev_addr, rc);
                return (FAILED);
            } else {
                 i2c_dev->fp = cavium_i2c_fd;
            }
        }
    } 
    
    return (rc);
}

/******************************************************************************
 *
 * Function   : read_i2c_reg
 * Description: Read i2c device register
 * Inputs     : *dev - pointer to the I2C device
 *              *data - pointer to the data of the register
 *              offset - I2C device register offset
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int read_i2c_reg (n2g_i2c_dev_t *dev, uchar *data, uint offset, uchar data_size)
{
    uint32 rc = FAILED;
    n2g_i2c_if_t i2c_if;
    
    /* Clear error buffer */
    memset(i2c_err_buf, 0, sizeof(i2c_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.size = data_size;
    i2c_if.buf = (char *)data;
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;
    i2c_if.offset = offset;

    rc = api_mb_i2c_read(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);

    if (rc != PASSED) {
        sprintf(i2c_err_buf, "%s:%d I2C read failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */

    return (PASSED);
}

/******************************************************************************
 *
 * Function   : write_i2c_reg
 * Description: Write i2c register
 * Inputs     : *dev - pointer to the I2C device
 *              *data - pointer to the data of the register
 *              offset - I2C device register offset
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int write_i2c_reg (n2g_i2c_dev_t *dev, uchar *data, uint offset, uchar data_size)
{
    uint32 rc = FAILED;
    n2g_i2c_if_t i2c_if;

    /* Clear error buffer */
    memset(i2c_err_buf, 0, sizeof(i2c_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.size = data_size;
    i2c_if.buf = (char *)data;
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;
    i2c_if.offset = offset;
    
    rc = api_mb_i2c_write(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
    if (rc != PASSED) {
        sprintf(i2c_err_buf, "%s:%d I2C write failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */

    return (PASSED);
}

/******************************************************************************
 *
 * Function   : pca9557_power_margin_ctrl
 * Description: By setting PCA9557 to control DC/DC output voltage
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int pca9557_power_margin_ctrl (void)
{   
    unsigned long voltage, margin;
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    uchar buf, offset_val;
    i2c_slave_addr = CAVIUM_PCA9557;

    /* Open the Cavium I2C bus 0 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C0) == FAILED) {
        printf("Fail to open the i2c interface\n");
        return (FAILED);
    }

    offset_val = PCA9557_CONFIGURATION_VAL;
    /* Configure to output pin */
    if (write_i2c_reg(&i2c_dev, &offset_val, PCA9557_CONFIGURATION_REG, 
                      sizeof(pca9557)) != PASSED) {
        printf("Write val %x to reg %x failed\n", offset_val, 
               PCA9557_CONFIGURATION_REG);
        return (FAILED);
    }

    /* Read original val of output reg */
    if (read_i2c_reg(&i2c_dev, &buf, PCA9557_OUTPUT_REG, 
                     sizeof(pca9557)) != PASSED) {
        printf("Read reg %x fail\n", PCA9557_OUTPUT_REG);
        return (FAILED);
    }
    
    printf("Select Voltage Rail\n");
    printf("Enter 0 - 1.0V Rail\n");
    printf("Enter 1 - 1.1V Rail\n");
    printf("Enter 2 - 1.5V Rail\n");
    printf("Enter 3 - 3.3V Rail\n");
    fflush(stdout);
    voltage = gethex_answer("Enter Voltage Rail: ", 0, 0x0, 0x3);
    
    printf("\nSelect Margin\n");
    printf("Enter 0 - Normal\n");
    printf("Enter 1 - Margin High\n");
    printf("Enter 2 - Margin Low\n");
    fflush(stdout);
    margin = gethex_answer("Enter Voltage Rail: ", 0, 0x0, 2);
    
    switch (voltage) {
    case 0: /* 1.0 V */
        buf &= ~(PCA9557_VAL_0_1 | PCA9557_VAL_0_2);
        switch (margin) {
        case 0: /* Normal */
            break;
        case 1: /* High */
            buf |= PCA9557_VAL_0_1;
            break;
        case 2: /* Low */
            buf |= PCA9557_VAL_0_2;
            break;
        default :
            printf("Enter wrong val - %ld\n", margin);
            return (FAILED);
        }
        break;
    case 1: /* 1.1 V */
        buf &= ~(PCA9557_VAL_1_1 | PCA9557_VAL_1_2);
        switch (margin) {
        case 0: /* Normal */
            break;
        case 1: /* High */
            buf |= PCA9557_VAL_1_1;
            break;
        case 2: /* Low */
            buf |= PCA9557_VAL_1_2;
            break;
        default :
            printf("Enter wrong val - %ld\n", margin);
            return (FAILED);
        }
        break;
    case 2: /* 1.5 V */
        buf &= ~(PCA9557_VAL_2_1 | PCA9557_VAL_2_2);
        switch (margin) {
        case 0: /* Normal */
            break;
        case 1: /* High */
            buf |= PCA9557_VAL_2_1;
            break;
        case 2: /* Low */
            buf |= PCA9557_VAL_2_2;
            break;
        default :
            printf("Enter wrong val - %ld\n", margin);
            return (FAILED);
        }
        break;
    case 3: /* 3.3 V */
        buf &= ~(PCA9557_VAL_3_1 | PCA9557_VAL_3_2);
        switch (margin) {
        case 0: /* Normal */
            break;
        case 1: /* High */
            buf |= PCA9557_VAL_3_1;
            break;
        case 2: /* Low */
            buf |= PCA9557_VAL_3_2;
            break;
        default :
            printf("Enter wrong val - %ld\n", margin);
            return (FAILED);
        }
            break;
        default :
            cterr('f', 0, "Not support this mode - %ld", margin);
            return (FAILED);
    }

    /* Call the I2C common I2C api to write the register value */
    if (write_i2c_reg(&i2c_dev, &buf, PCA9557_OUTPUT_REG, sizeof(pca9557)) != PASSED) {
        printf("Write val %x to reg %x failed\n", offset_val, PCA9557_OUTPUT_REG);
        return (FAILED);
    }
    
    return (PASSED);
}
/* end of file */

/******************************************************************************
 *
 * Function   : pca9557_power_margin_status
 * Description: Show PCA9557 power margin status
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int pca9557_power_margin_status (void)
{   
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    uchar buf, mode_0, mode_1, mode_2, mode_3;
    i2c_slave_addr = CAVIUM_PCA9557;

    /* Open the Cavium I2C bus 0 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C0) == FAILED) {
        printf("Fail to open the i2c interface\n");
        return (FAILED);
    }

    /* Read original val of output reg */
    if (read_i2c_reg(&i2c_dev, &buf, PCA9557_OUTPUT_REG, 
                     sizeof(pca9557)) != PASSED) {
        printf("Read reg %x fail\n", PCA9557_OUTPUT_REG);
        return (FAILED);
    }

    printf("1.0V - ");
    mode_0 = buf & (PCA9557_VAL_0_1 | PCA9557_VAL_0_2);
    if (mode_0 == PCA9557_VAL_0_1) {
        printf("Margin high\n");
    } else if (mode_0 == PCA9557_VAL_0_2) {
        printf("Margin low\n");
    } else {
        printf("Margin normal\n");
    }

    printf("1.1V - ");
    mode_1 = buf & (PCA9557_VAL_1_1 | PCA9557_VAL_1_2);
    if (mode_1 == PCA9557_VAL_1_1) {
        printf("Margin high\n");
    } else if (mode_1 == PCA9557_VAL_1_2) {
        printf("Margin low\n");
    } else {
        printf("Margin normal\n");
    }

    printf("1.5V - ");
    mode_2 = buf & (PCA9557_VAL_2_1 | PCA9557_VAL_2_2);
    if (mode_2 == PCA9557_VAL_2_1) {
        printf("Margin high\n");
    } else if (mode_2 == PCA9557_VAL_2_2) {
        printf("Margin low\n");
    } else {
        printf("Margin normal\n");
    }

    printf("3.3V - ");
    mode_3 = buf & (PCA9557_VAL_3_1 | PCA9557_VAL_3_2);
    if (mode_3 == PCA9557_VAL_3_1) {
        printf("Margin high\n");
    } else if (mode_3 == PCA9557_VAL_3_2) {
        printf("Margin low\n");
    } else {
        printf("Margin normal\n");
    }
    return (PASSED);
}
/* end of file */
/******** History ********
*---------------------------------------------------
* $Log: platform_i2c.c,v $
* Revision 1.3  2013/11/26 08:40:39  hroni
* fix compiler warning
*
* Revision 1.2  2013/10/08 08:48:30  tirawan
* Woodlawn collapsed to main trunk
*
* Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
* Branch into woodlawn-branch2 and port woodlawn code
*
* Revision 1.1.2.1  2013/04/24 10:37:24  tirawan
* Initial check-in for woodlawn linux code
*
* Revision 1.5  2013/04/09 09:08:07  leslie
* Modify PCA9557 power margin ctrl.
*
* Revision 1.4  2013/04/08 08:56:32  leslie
* Add PCA9557 power margin ctrl and show status functions
*
* Revision 1.3  2013/03/27 08:45:05  kuangik
* Code cleanup
*
* Revision 1.10  2012/10/24 10:43:16  leslie
* Fix and clean up code.
*
* Revision 1.9  2012/08/30 06:37:05  leslie
* Fix read/write reg lib.
*
* Revision 1.7  2012/08/18 02:47:12  leslie
* Pass I2C bus argument to open_i2c function to decide open i2c-octeon 0 or 1
*
* Revision 1.6  2012/08/16 07:12:18  evanli
* modify I2C BUS to 1
*
* Revision 1.5  2012/08/13 12:25:47  leslie
* Fix and assign the size of I2C struct.
*
* Revision 1.4  2012/08/03 10:16:56  evanli
* Mapping to latest O2 source code on 20120726
*
* Revision 1.2  2012/07/19 06:45:00  leslie
* Add I2C platform code library.
*
* Revision 1.1  2012/03/26 07:35:15  kody
* Add I2C platform code library.
*
*---------------------------------------------------
* $Endlog$
*/
