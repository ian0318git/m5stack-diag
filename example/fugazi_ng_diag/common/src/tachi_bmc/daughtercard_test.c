/* $Id: daughtercard_test.c,v 1.5 2017/04/13 01:02:03 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/daughtercard_test.c,v $
 *------------------------------------------------------------------
 *
 * daughtercard_test.c
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "mon_plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "platform_fru.h"
#include "diag_raid_test.h"
#include "diag_testcard_test.h"
#include "daughtercard_test.h"
#include "diag_i2c_lib.h"
#include "i2c_api.h"
#include "diag_i2c_api.h"
#include "diag_i2c_test.h"
#include "platform_i2c.h"
#include "diag_fpga_lib.h"
#include "diag_raid_lib.h"

int daughtercard_test (int flag)
{
    int retval = FAILED;
    set_nios_mode(NIOS_DISABLE_MODE);
    uint8_t pca9557_val;

    /* Check if this is test card 
     * (ACT2 is not populated on test card by default) 
     */
    if (!diag_i2c_ping(CPU_I2C5, (TEST_CARD_I2C_ADDR << 1), 0)) {
        if (diag_testcard_build_test(flag) == PASSED) {
            retval = PASSED;
        }
    /* Read any offset to check if CPLD chip is on, if the chip
     * can be read means that ISP card has been plugged in 
     */    
    } else if (platform_pca9557_i2c_r(0x0,&pca9557_val ) == PASSED) {
        if (diag_raidcard_build_test(flag) == PASSED ) {
            retval = PASSED;
        }
    } else { 
        printf("ISP CARD is Vacant");
        retval = FAILED;    
    }
    
    set_nios_mode(NIOS_DIAG_MODE);
    return (retval);
}

int daughtercard_iface_test (void)
{   
    int retval = FAILED;
    set_nios_mode(NIOS_DISABLE_MODE);
    uint8_t pca9557_val;

    /* Check if this is test card
     * (ACT2 is not populated on test card by default)
     */
    if (!diag_i2c_ping(CPU_I2C5, (TEST_CARD_I2C_ADDR << 1), 0)) {
        /* DO ISP Testcard IO TEST */
        retval = diag_testcard_io_test();
    } else if (platform_pca9557_i2c_r(0x0,&pca9557_val ) == PASSED) {
        /* Do RAID IO TEST */ 
        retval = diag_raid_io_test();
    } else {
        printf("ISP CARD is Vacant");
        retval = FAILED;   
    }

    set_nios_mode(NIOS_DIAG_MODE);
    return (retval);
}

/*---------------------------------------------------------------
$Log: daughtercard_test.c,v $
Revision 1.5  2017/04/13 01:02:03  haohsu
Modify ISP Card function for Tachi

Revision 1.4  2017/04/10 01:19:53  haohsu
Modify RaidCard function dor Tachi

Revision 1.3  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.2  2016/04/20 11:25:26  benchen2
add tachi fru portion

Revision 1.1.2.5  2016/03/26 05:27:53  benchen2
add raid io interface test

Revision 1.1.2.4  2016/03/10 05:39:05  uid421098
Add ISP test card io test

Revision 1.1.2.3  2016/01/29 02:47:22  benchen2
add disable nios feature

Revision 1.1.2.2  2016/01/11 10:28:15  tirawan
Add Test card menu to run FPGA i2c register test, btb test from x86 and Lewis

Revision 1.1.2.1  2015/11/13 07:19:23  benchen2
Add raid card entrance menu

$Endlog$
*/

