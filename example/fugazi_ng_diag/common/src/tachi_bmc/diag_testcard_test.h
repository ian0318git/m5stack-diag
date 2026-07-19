/* $Id: diag_testcard_test.h,v 1.2 2016/04/20 11:25:26 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_testcard_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_testcard_test.h
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef DIAG_TESTCARD_TEST_H_
#define DIAG_TESTCARD_TEST_H_

#define FPGA_SCRATCHPAD_REG_OFFSET                  (0x20)

extern int diag_testcard_build_test(int);
extern int diag_testcard_io_test(void);

#endif /* DIAG_TESTCARD_TEST_H_ */
/*---------------------------------------------------------------
$Log: diag_testcard_test.h,v $
Revision 1.2  2016/04/20 11:25:26  benchen2
add tachi fru portion

Revision 1.1.2.2  2016/03/10 05:39:05  uid421098
Add ISP test card io test

Revision 1.1.2.1  2016/01/11 10:50:59  tirawan
Add for the first time


$Endlog$
*/

