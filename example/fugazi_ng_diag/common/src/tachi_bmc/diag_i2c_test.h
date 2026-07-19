/* $Id: diag_i2c_test.h,v 1.2 2016/04/20 11:25:24 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_i2c_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_test.h - Header file for I2C test functions
 *
 * June 2015, Times Huang ported from mfgdiag
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_I2C_TEST__
#define __DIAG_I2C_TEST__

#include "types.h"

#define ONBOARD_DEV         0x0
#define REMOVABLE_TPM_DEV   0x1
#define REMOVABLE_HDD_DEV   0x2
#define REMOVABLE_DIMM_DEV  0x3
#define REMOVABLE_MEZZ_DEV  0x4

typedef struct i2c_table_s {
    char *dev_name;
    uint8_t bus;
    uint8_t addr; /* 8 bit i2c addr */
    char *desc;
    uint8_t removable; /* (0 - on board) | (1 - tpm) | (2 - hdd) | (3 - dimm) | (4 - mezz) */
    uint8_t intel_on; /* indicate whether this device requires Intel up or not */
    uint8_t mask_bit; /* indicate which hdd or dimm */
    uint8_t ctrl; /* For FPGA controller number */
} i2c_table_t;

extern int diag_i2c_scan_test(void);
extern int diag_intel_i2c_scan_test(void);
extern int diag_lewis_i2c_scan_test(void);

#endif /* __DIAG_I2C_TEST__ */

/*---------------------------------------------------------------
$Log: diag_i2c_test.h,v $
Revision 1.2  2016/04/20 11:25:24  benchen2
add tachi fru portion

Revision 1.1.2.4  2016/03/03 09:46:37  jimmyya
add GESW I2C test

Revision 1.1.2.3  2016/01/18 06:55:06  benchen2
separate i2c scan test

Revision 1.1.2.2  2015/12/16 01:55:53  huanngo
Add support for FPGA I2C device scan utility

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/
