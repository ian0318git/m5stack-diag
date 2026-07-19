/* $Id: diag_i2c_test.c,v 1.3 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_i2c_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_test.c - I2C test functions
 *
 * June 2015, Times Huang ported from mfgdiag
 *
 * Copyright (c) 2015-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include "common.h"
#include "diag_i2c_lib.h"
#include "diag_i2c_test.h" 
#include "error.h"
#include "platform_fru.h"
#include <stdlib.h>
#include <fcntl.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"

int diag_i2c_scan_test(void);
extern int diag_fpga_i2c_scan(void);
static int display_intel_i2c_scan_test_reg(void);

static i2c_table_t platform_i2c_table[] = {
    /* Dev on Bus 0 - PECI interface for Shedir */
    {"PCA9541", 0, 0xE0, "I2C dual master selector", ONBOARD_DEV, 0, 0, 0},
    
    /* Dev on Bus 2 */
	/*
    {"98DX4235", 2, 0xC2, "Cetus Lan Switch", ONBOARD_DEV, 0, 0, 0},
    {"i350", 2, 0x92, "Copper/SFP LAN PHY", ONBOARD_DEV, 0, 0, 0},
    {"i210", 2, 0x94, "Management PHY", ONBOARD_DEV, 0, 0, 0},
    {"X710", 2, 0x96, "Fortville vNIC", ONBOARD_DEV, 0, 0, 0}, */
    
    /* Dev on Bus 3 - BRCM GE LOM */
    {"RTC DS1337", 3, 0xD0, "Real time clock", ONBOARD_DEV, 0, 0, 0},
    {"AT24C02C", 3, 0xAE, "FRU ID EEPROM", ONBOARD_DEV, 0, 0, 0},
    {"IDT 8T49N287", 3, 0xF8, "Clock Generator", ONBOARD_DEV, 0, 0, 0},
    
    /* Dev on Bus 7 */
    {"PCA9543", 7, 0xE0, "SFP I2C 2-channel bus switch", ONBOARD_DEV, 0, 0, 0},
    /* end of lists */
    {NULL, 0, 0xFF, NULL, 0, 0},
};

static i2c_table_t intel_i2c_table[] = {
    /* Dev on Bus 1 */
    {"BDX-DE CPU", 1, 0x2E, "CPU SM Link1", ONBOARD_DEV, 0, 0, 0},
    
    /* Dev on Bus 2 */
    {"PI7C9X2G304", 2, 0xD0, "PCIe Switch", ONBOARD_DEV, 0, 0, 0},
    
    /* Dev on Bus 4 - Shedir SM Link to Intel Patsburg */
    {"BDX-DE CPU", 4, 0x2C, "CPU SM Link0", ONBOARD_DEV, 0, 0, 0},
    
    /* end of lists */
    {NULL, 0, 0xFF, NULL, 0, 0},
};

static i2c_table_t lewis_i2c_table[] = {
    {"98DX4235", 2, 0xC2, "Cetus Lan Switch", ONBOARD_DEV, 0, 0, 0},
};

int diag_i2c_scan_test (void)
{
    i2c_table_t *dev = NULL;
    int index = 0;
    int retval = PASSED;
    
    testname("I2C Scan");
    prpass(testpass, "I2C Scan Test");
    
    printf("\n");
    printf("DEV_NAME         BUS  ADDRESS   DESCRIPTION                  STATUS\n");
    
    do {
        dev = &platform_i2c_table[index];
        
        printf("%-15s   %x    0x%02x     %-30s ", dev->dev_name, dev->bus,
                                                  dev->addr, dev->desc);

        if (diag_i2c_ping(dev->bus, dev->addr, 0)) {
            printf("ERR\n");
            retval = FAILED;
        } else {
            printf("OK\n");
        }        
        index++;
    } while (platform_i2c_table[index].dev_name);


    retval |= diag_fpga_i2c_scan();

    if (retval == FAILED) {
        cterr('f', 0, "I2C Scan failed");
    }

    return (retval);
}

static void
add_diag_intel_i2c_scan_test_err_report(void)
{
    fru_table_offset = INTEL_I2C;
    platform_fru_table[INTEL_I2C].pid_string = intel_i2c;
    platform_fru_table[INTEL_I2C].location_string = intel_i2c_loc;
    cterr_add_component("MB", "FPGA/Rangeley", "I2C");
    cterr_add_reg_dump((PFV)display_intel_i2c_scan_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please ensure INTEL already power on",
    		"Please use i2c utility to make sure i2c is working");

}
 
static int display_intel_i2c_scan_test_reg(void)
{
    cterr_db_print("Test by i2c_drv driver, no register dump");
    return (PASSED);
}


int diag_intel_i2c_scan_test (void)
{
    i2c_table_t *dev = NULL;
    int index = 0;
    int retval = PASSED;
    
    if (get_enhance_err_flag()) {
            add_diag_intel_i2c_scan_test_err_report();
    }

    testname("Intel I2C Scan");
    prpass(testpass, "Intel I2C Scan Test");
    
    printf("\n");
    printf("DEV_NAME         BUS  ADDRESS   DESCRIPTION                  STATUS\n");
    
    do {
        dev = &intel_i2c_table[index];
        
        printf("%-15s   %x    0x%02x     %-30s ", dev->dev_name, dev->bus,
                                                  dev->addr, dev->desc);

        if (diag_i2c_ping(dev->bus, dev->addr, 0)) {
            printf("ERR\n");
            retval = FAILED;
        } else {
            printf("OK\n");
        }        
        index++;
    } while (intel_i2c_table[index].dev_name);
    
    if (retval == FAILED) {
        cterr('f', 0, "intel I2C Scan failed");
    }

    return (retval);
}

static void
add_diag_lewis_i2c_scan_test_err_report(void)
{
    fru_table_offset = GESW_98DX_I2C;
    platform_fru_table[GESW_98DX_I2C].pid_string = gesw_98DX_i2c;
    platform_fru_table[GESW_98DX_I2C].location_string = gesw_98DX_i2c_loc;
    cterr_add_component("MB", "FPGA/Rangeley", "I2C");
    cterr_add_debug("Please ensure Lewis already power on",
                "Please use i2c utility to make sure i2c is working");

}

int diag_lewis_i2c_scan_test (void)
{   
    i2c_table_t *dev = NULL;
    int index = 0;
    int retval = PASSED;
   
    if (get_enhance_err_flag()) {  
            add_diag_lewis_i2c_scan_test_err_report();
    }

    testname("Lewis I2C Scan");
    prpass(testpass, "Lewis I2C Scan Test");

    printf("\n");
    printf("DEV_NAME         BUS  ADDRESS   DESCRIPTION                  STATUS\n");

    dev = &lewis_i2c_table[index];

    printf("%-15s   %x    0x%02x     %-30s ", dev->dev_name, dev->bus,
                                              dev->addr, dev->desc);

    if (diag_i2c_ping(dev->bus, dev->addr, 0)) {
        printf("ERR\n");
        retval = FAILED;
    } else {
        printf("OK\n");
    }

    if (retval == FAILED) {
        cterr('f', 0, "Lewis I2C Scan failed");
    }

    return (retval);
}


/*---------------------------------------------------------------
$Log: diag_i2c_test.c,v $
Revision 1.3  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.2.14.2  2016/11/30 13:32:28  hondwang
Fix build image issue with enhance error message

Revision 1.2  2016/04/20 11:25:24  benchen2
add tachi fru portion

Revision 1.1.2.11  2016/03/03 09:46:37  jimmyya
add GESW I2C test

Revision 1.1.2.10  2016/02/26 09:00:22  hondwang
add intel enhance error message, pci bus scan

Revision 1.1.2.9  2016/01/20 22:55:45  huanngo
Fix FPGA I2C scan by removing unexisting devices in the list

Revision 1.1.2.8  2016/01/18 06:55:06  benchen2
separate i2c scan test

Revision 1.1.2.7  2016/01/14 08:16:45  benchen2
fix MB ACT2 issue

Revision 1.1.2.6  2015/12/16 01:55:53  huanngo
Add support for FPGA I2C device scan utility

Revision 1.1.2.5  2015/10/01 07:24:15  tirawan
Correct M/B Test name

Revision 1.1.2.4  2015/09/17 03:48:27  benchen2
include err

Revision 1.1.2.3  2015/09/17 03:09:23  benchen2
add testname

Revision 1.1.2.2  2015/08/16 06:01:01  tirawan
Tachi bring up fix: SPI Flash Test, I2C Library for RTC Test, I2C scan Test, CPU ID fix for PECI test

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/
