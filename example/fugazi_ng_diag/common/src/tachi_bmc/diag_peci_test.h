/* $Id: diag_peci_test.h,v 1.2 2016/04/20 11:25:30 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_peci_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_peci_test.h - Header file for PECI Test
 *
 * June 2015, Times Huang ported from mfgdiag
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_PECI_TEST__
#define __DIAG_PECI_TEST__

#define INTEL_BROADWELL_CPU_MODEL           (0x06)
#define INTEL_BW_FAMILY_ID                  (0x06)

#define PECI_DEV_ADDR                       (0x30)

extern int diag_peci_test(void);

#endif /* __DIAG_PECI_TEST__ */

/*---------------------------------------------------------------
$Log: diag_peci_test.h,v $
Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/08/16 06:01:01  tirawan
Tachi bring up fix: SPI Flash Test, I2C Library for RTC Test, I2C scan Test, CPU ID fix for PECI test

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/

